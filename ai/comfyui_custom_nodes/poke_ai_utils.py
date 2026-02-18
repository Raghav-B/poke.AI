"""
poke.AI utility nodes for ComfyUI.

Install: copy this file to ComfyUI/custom_nodes/poke_ai_utils.py
then restart ComfyUI.
"""


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


NODE_CLASS_MAPPINGS = {
    "NumberToString": NumberToString,
}

NODE_DISPLAY_NAME_MAPPINGS = {
    "NumberToString": "Number to String",
}
