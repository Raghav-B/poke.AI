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
                "decimals": ("INT", {"default": 4, "min": 0, "max": 10}),
            },
        }

    RETURN_TYPES = ("STRING",)
    FUNCTION = "execute"
    CATEGORY = "poke.AI/utils"

    def execute(self, number, prefix="Score: ", decimals=4):
        return (f"{prefix}{number:.{decimals}f}",)


NODE_CLASS_MAPPINGS = {
    "NumberToString": NumberToString,
}

NODE_DISPLAY_NAME_MAPPINGS = {
    "NumberToString": "Number to String",
}
