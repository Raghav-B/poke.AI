"""
poke.AI utility nodes for ComfyUI.

Install: copy this file to ComfyUI/custom_nodes/poke_ai_utils.py
then restart ComfyUI.

Dependencies (already satisfied if Strimmlarns Aesthetic Score
and ComfyUI-Face-Comparator are installed):
  pip install open-clip-torch insightface onnxruntime-gpu
"""

import numpy as np
import torch
import torch.nn as nn

# ---------------------------------------------------------------------------
# Aesthetic scoring internals (same MLP as improved-aesthetic-predictor)
# ---------------------------------------------------------------------------

class _AestheticMLP(nn.Module):
    """MLP head: 768-dim CLIP embedding -> 1 aesthetic score."""

    def __init__(self, input_size=768):
        super().__init__()
        self.layers = nn.Sequential(
            nn.Linear(input_size, 1024),
            nn.Dropout(0.2),
            nn.Linear(1024, 128),
            nn.Dropout(0.2),
            nn.Linear(128, 64),
            nn.Dropout(0.1),
            nn.Linear(64, 16),
            nn.Linear(16, 1),
        )

    def forward(self, x):
        return self.layers(x)


def _l2_normalize(a, axis=-1):
    norms = np.atleast_1d(np.linalg.norm(a, 2, axis))
    norms[norms == 0] = 1
    return a / np.expand_dims(norms, axis)


# ---------------------------------------------------------------------------
# Singleton caches (loaded once, reused across executions)
# ---------------------------------------------------------------------------

_clip_model = None
_clip_preprocess = None
_face_app = None


def _get_clip(device="cuda"):
    global _clip_model, _clip_preprocess
    if _clip_model is None:
        import open_clip
        _clip_model, _, _clip_preprocess = open_clip.create_model_and_transforms(
            "ViT-L-14", pretrained="openai", device=device,
        )
    return _clip_model, _clip_preprocess


def _get_face_app():
    global _face_app
    if _face_app is None:
        from insightface.app import FaceAnalysis
        _face_app = FaceAnalysis(
            name="buffalo_l",
            providers=["CUDAExecutionProvider", "CPUExecutionProvider"],
        )
        _face_app.prepare(ctx_id=0, det_size=(640, 640))
    return _face_app


# ---------------------------------------------------------------------------
# ImageCurator node
# ---------------------------------------------------------------------------

class ImageCurator:
    """
    Batch-aware image curator for LoRA dataset generation.

    For each image in the batch:
      1. Calculates aesthetic score (CLIP + MLP, scale 0-10)
      2. Compares face similarity with a reference (InsightFace cosine)
    Only images that pass BOTH thresholds are forwarded to the output.
    Set a threshold to 0 to skip that check.
    """

    @classmethod
    def INPUT_TYPES(cls):
        return {
            "required": {
                "images": ("IMAGE",),
                "reference_image": ("IMAGE",),
                "aesthetic_model": ("AESTHETIC_MODEL",),
            },
            "optional": {
                "aesthetic_threshold": (
                    "FLOAT",
                    {"default": 6.0, "min": 0.0, "max": 10.0, "step": 0.1},
                ),
                "face_threshold": (
                    "FLOAT",
                    {"default": 0.65, "min": 0.0, "max": 1.0, "step": 0.01},
                ),
            },
        }

    RETURN_TYPES = ("IMAGE", "STRING",)
    RETURN_NAMES = ("approved_images", "report",)
    OUTPUT_NODE = True
    FUNCTION = "curate"
    CATEGORY = "poke.AI/curation"

    # ---- aesthetic scoring ----

    @staticmethod
    def _score_aesthetic(pil_image, mlp, clip_model, preprocess, device="cuda"):
        from PIL import Image
        img_t = preprocess(pil_image).unsqueeze(0).to(device)
        with torch.no_grad():
            features = clip_model.encode_image(img_t)
        emb = _l2_normalize(features.cpu().numpy())
        with torch.no_grad():
            pred = mlp(torch.from_numpy(emb).to(device).float())
        return float(pred[0])

    # ---- face similarity ----

    @staticmethod
    def _face_embedding(img_bgr, face_app):
        faces = face_app.get(img_bgr)
        if not faces:
            return None
        return faces[0].embedding

    @staticmethod
    def _cosine_similarity(a, b):
        return float(np.dot(a, b) / (np.linalg.norm(a) * np.linalg.norm(b)))

    # ---- main ----

    def curate(
        self,
        images,
        reference_image,
        aesthetic_model,
        aesthetic_threshold=6.0,
        face_threshold=0.65,
    ):
        import cv2
        from PIL import Image

        device = "cuda" if torch.cuda.is_available() else "cpu"
        batch_size = images.shape[0]

        # --- Load models once for entire batch ---
        clip_model, preprocess = _get_clip(device)

        mlp = _AestheticMLP(768)
        # aesthetic_model is already a state_dict loaded by LoadAesteticModel
        mlp.load_state_dict(aesthetic_model)
        mlp.to(device).eval()

        face_app = _get_face_app()

        # --- Reference face embedding (computed once) ---
        ref_np = (reference_image[0] * 255).to(torch.uint8).cpu().numpy()
        ref_bgr = cv2.cvtColor(ref_np, cv2.COLOR_RGB2BGR)
        ref_emb = self._face_embedding(ref_bgr, face_app)

        # --- Process each image ---
        approved = []
        lines = [
            "=== ImageCurator Report ===",
            f"Batch: {batch_size} | aesthetic >= {aesthetic_threshold} | face >= {face_threshold}",
            "",
        ]

        for i in range(batch_size):
            img_tensor = images[i]
            np_img = (img_tensor * 255).to(torch.uint8).cpu().numpy()
            pil_img = Image.fromarray(np_img, mode="RGB")
            bgr_img = cv2.cvtColor(np_img, cv2.COLOR_RGB2BGR)

            # Aesthetic
            aes = self._score_aesthetic(pil_img, mlp, clip_model, preprocess, device)
            aes_ok = aes >= aesthetic_threshold or aesthetic_threshold == 0.0

            # Face
            if ref_emb is not None and face_threshold > 0.0:
                gen_emb = self._face_embedding(bgr_img, face_app)
                if gen_emb is not None:
                    sim = self._cosine_similarity(gen_emb, ref_emb)
                else:
                    sim = 0.0
                face_ok = sim >= face_threshold
            else:
                sim = -1.0
                face_ok = True  # skip face check

            passed = aes_ok and face_ok
            tag = "OK" if passed else "X"

            face_str = f"{sim:.3f}" if sim >= 0 else "n/a"
            lines.append(
                f"  #{i+1}: aesthetic={aes:.2f} {'OK' if aes_ok else 'X'} | "
                f"face={face_str} {'OK' if face_ok else 'X'} -> {tag}"
            )

            if passed:
                approved.append(img_tensor)

        # --- Cleanup ---
        del mlp

        # --- Build output ---
        lines.append("")
        lines.append(f"Result: {len(approved)}/{batch_size} approved")
        report = "\n".join(lines)

        if approved:
            out = torch.stack(approved)
        else:
            out = torch.zeros(1, 8, 8, 3)
            report += "\n(!) Nenhuma imagem aprovada. Placeholder retornado."

        return (out, report)


# ---------------------------------------------------------------------------
# NumberToString (utility)
# ---------------------------------------------------------------------------

class NumberToString:
    """Converts a NUMBER to STRING for display in ShowText nodes."""

    @classmethod
    def INPUT_TYPES(cls):
        return {
            "required": {
                "number": ("NUMBER",),
            },
            "optional": {
                "prefix": ("STRING", {"default": "Score: "}),
                "decimals": ("INT", {"default": 2, "min": 0, "max": 10}),
                "divisor": ("FLOAT", {"default": 1.0, "min": 0.001, "max": 10000.0, "step": 0.1}),
            },
        }

    RETURN_TYPES = ("STRING",)
    FUNCTION = "execute"
    CATEGORY = "poke.AI/utils"

    def execute(self, number, prefix="Score: ", decimals=2, divisor=1.0):
        value = number / divisor
        return (f"{prefix}{value:.{decimals}f}",)


# ---------------------------------------------------------------------------
# Registration
# ---------------------------------------------------------------------------

NODE_CLASS_MAPPINGS = {
    "ImageCurator": ImageCurator,
    "NumberToString": NumberToString,
}

NODE_DISPLAY_NAME_MAPPINGS = {
    "ImageCurator": "Image Curator (poke.AI)",
    "NumberToString": "Number to String",
}
