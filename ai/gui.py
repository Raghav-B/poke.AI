import tkinter as tk
from PIL import Image, ImageTk
import cv2
import numpy as np

from standalone_backend import poke_ai

class gui:
    def __init__(self, window, window_title):
        self.window = window
        self.window.title(window_title)

        self.game_window_size = {"top": 0, "left": 0, "width": 720, "height": 480}
        self.model_path = "../object_detection/keras-retinanet/inference_graphs/map_detector.h5" # Model to be used for detection
        self.labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit"} # Labels to draw
        self.pa = poke_ai(self.model_path, self.labels_to_names, self.game_window_size)

        self.main_frame = tk.Frame(self.window, width=1440, height=720)
        self.main_frame.grid(row=1, column=0, columnspan=4, padx=5, pady=5)


        self.detect_frame = tk.Label(self.main_frame, width=720, height=720)
        self.detect_frame.grid(row=1,column=0, columnspan=2, padx=5, pady=5)
        self.df_label = tk.Label(self.main_frame, text="Detection Screen", font=("Helvetica", 12))
        self.df_label.grid(row=0, column=0, columnspan=2, padx=5, pady=5)

        self.map_frame = tk.Label(self.main_frame, width=720, height=720)
        self.map_frame.grid(row=1, column=2, columnspan=2, padx=5, pady=5)
        self.mf_label = tk.Label(self.main_frame, text="Explored Map", font=("Helvetica", 12))
        self.mf_label.grid(row=0, column=2, columnspan=2, padx=5, pady=5)

        self.is_paused = True
        self.initial = True
        self.pause_button_text = tk.StringVar()
        self.pause_button_text.set("Start")
        self.pause_button = tk.Button(self.main_frame, textvariable=self.pause_button_text, font=("Helvetica", 12), \
            command=self.pause_ai)
        self.pause_button.grid(row=2, column=0, columnspan=4, padx=5, pady=5)

        self.update()
        self.window.mainloop()
    
    def update(self):
        if not (self.is_paused == True and self.initial == False):
            frame, map_grid = self.pa.run_step()

            frame = cv2.resize(frame, (720,720), interpolation=cv2.INTER_LINEAR)
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGBA)
            frame = Image.fromarray(frame)
            frame = ImageTk.PhotoImage(image=frame)
            self.detect_frame.imgtk = frame
            self.detect_frame.configure(image=frame)
            
            # Padding map to make it square
            width = map_grid.shape[1]
            height = map_grid.shape[0]
            map_grid = map_grid[:,:,:3]
            padding = 0
            if height < width:
                padding = int((width - height) / 2)
                frame = cv2.copyMakeBorder(map_grid, padding, padding, 0, 0, cv2.BORDER_CONSTANT, (0, 0, 0))
            elif height > width:
                padding = int((height - width) / 2)
                frame = cv2.copyMakeBorder(map_grid, 0, 0, padding, padding, cv2.BORDER_CONSTANT, (0, 0, 0))

            map_grid = cv2.resize(map_grid, (720,720), interpolation=cv2.INTER_NEAREST)
            map_grid = cv2.cvtColor(map_grid, cv2.COLOR_BGR2RGBA)
            map_grid = Image.fromarray(map_grid)
            map_grid = ImageTk.PhotoImage(image=map_grid)
            self.map_frame.imgtk = map_grid
            self.map_frame.configure(image=map_grid)

        self.initial = False
        self.main_frame.after(1, self.update)
    
    def pause_ai(self):
        self.is_paused = not self.is_paused
        if (self.pause_button_text.get() == "Start"):
            self.pause_button_text.set("Stop")
        else:
            self.pause_button_text.set("Start")

if __name__ == "__main__":
    gui(tk.Tk(), "poke.AI")