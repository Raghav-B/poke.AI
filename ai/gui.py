import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
from PIL import Image, ImageTk
import cv2
import numpy as np
import threading

from standalone_backend import poke_ai

"""
Stuff to add to GUI. 
- Speed slider

- Status listbox which can be double-clicked to open image at that point in time
Possible statuses:
- Finding frontier: (Frontier location)
- Moving to frontier: (Actions required to move there)
- Move: Up, Down, Left Right:
- In battle: (Model output scores after this battle)
- 
- Collision, continuing movement to frontier 
- Frontier obstructed, finding new frontier (Frontier location)
- 


- Matplotlib graph to show the score for each move slowly converge


- Battle AI training status, without output weights
- Turn on/off DQNN training: self.pa.bat_ai.continue_training = False
- 

- Try to get custom running speed version to work

Data to show:
- Move selected:
- Battle won/lost
- Battle ended
- 

"""

class gui:
    def __init__(self, window, window_title):
        self.window = window
        self.window.title(window_title)
        self.window.resizable(False, False)

        self.game_window_size = {"top": 0, "left": 0, "width": 720, "height": 480}
        self.model_path = "../object_detection/keras-retinanet/inference_graphs/map_detector.h5"#resnet50_csv_13.h5" # Model to be used for detection
        self.labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit", 6: "wall", 7:"grass"} # Labels to draw
        self.attacks = ["Scratch", "Growl", "Focus Energy", "Ember"]
        self.pa = poke_ai(self.model_path, self.labels_to_names, self.game_window_size)
        #self.video = cv2.VideoCapture(0)
        self.cur_map_grid = None
        self.map_num = 0

        treeview_style = ttk.Style()
        treeview_style.configure("Treeview", rowheight=25)
        treeview_style.configure("Treeview", font=("Helvetica", 7))

        ### MAP LEGEND ###
        self.legend_label = tk.Label(self.window, text="Map Legend", font=("Helvetica", 12))
        self.legend_label.grid(row=0, column=12, padx=5, pady=5)
        self.legend = tk.Canvas(self.window, width=300, height=375)
        self.legend.grid(row=1, column=12, rowspan=3, padx=1, pady=5, sticky="n")
        self.legend.create_rectangle(3, 5, 300, 375, fill="#FFFFFF", outline="#000000", width=2)
        # Agent
        self.legend.create_rectangle(15, 15, 45, 45, width=1, outline="#000000", fill="#00FF95")
        self.legend.create_text(55, 30, anchor="w", font=("Helvetica", 10), text="Agent")
        # Target Frontier
        self.legend.create_rectangle(15, 60, 45, 90, width=1, outline="#000000", fill="#FF00EA")
        self.legend.create_text(55, 75, anchor="w", font=("Helvetica", 10), text="Target Frontier")
        # Wall / Boundary
        self.legend.create_rectangle(15, 105, 45, 135, width=1, outline="#000000", fill="#696969")
        self.legend.create_text(55, 120, anchor="w", font=("Helvetica", 10), text="Boundary")
        # NPC
        self.legend.create_rectangle(15, 150, 45, 180, width=1, outline="#000000", fill="#F58742")
        self.legend.create_text(55, 165, anchor="w", font=("Helvetica", 10), text="NPC")
        # House
        self.legend.create_rectangle(15, 195, 45, 225, width=1, outline="#000000", fill="#66391E")
        self.legend.create_text(55, 210, anchor="w", font=("Helvetica", 10), text="Building")
        # Pokemon Center
        self.legend.create_rectangle(15, 240, 45, 270, width=1, outline="#000000", fill="#FF0000")
        self.legend.create_text(55, 255, anchor="w", font=("Helvetica", 10), text="Pokemon Center")
        # Pokemart
        self.legend.create_rectangle(15, 285, 45, 315, width=1, outline="#000000", fill="#0000FF")
        self.legend.create_text(55, 300, anchor="w", font=("Helvetica", 10), text="Pokemart")
        # Gym
        self.legend.create_rectangle(15, 330, 45, 360, width=1, outline="#000000", fill="#1E6660")
        self.legend.create_text(55, 345, anchor="w", font=("Helvetica", 10), text="Gym")

        ### DETECTION SCREEN ###
        self.df_label = tk.Label(self.window, text="Detection Screen", font=("Helvetica", 12))
        self.df_label.grid(row=0, column=0, columnspan=6, padx=5, pady=5)
        self.detect_frame_outer = tk.Label(self.window, width=720, height=720)
        self.detect_frame_outer.grid(row=1, column=0, rowspan=6, columnspan=6, padx=5, pady=5)
        self.detect_frame = tk.Label(self.detect_frame_outer, width=720, height=720)
        self.detect_frame.grid(row=1, column=0, rowspan=6, columnspan=6)#, padx=5, pady=5)

        ### EXPLORED MAP ###
        self.mf_label = tk.Label(self.window, text="Explored Map", font=("Helvetica", 12))
        self.mf_label.grid(row=0, column=6, columnspan=6, padx=5, pady=5)
        self.map_frame_outer = tk.Label(self.window, width=720, height=720)
        self.map_frame_outer.grid(row=1, column=6, rowspan=6, columnspan=6, padx=5, pady=5)
        self.map_frame = tk.Label(self.map_frame_outer, width=720, height=720)
        self.map_frame.grid(row=1, column=6, rowspan=6, columnspan=6)#, padx=5, pady=5)

        ### OPTIONS BUTTONS ###
        self.is_paused = True
        self.initial = True
        self.pause_button_text = tk.StringVar()
        self.pause_button_text.set("Start")
        self.pause_button = tk.Button(self.window, textvariable=self.pause_button_text, font=("Helvetica", 10), \
            command=self.pause_ai, borderwidth=3)
        self.pause_button.grid(row=4, column=12, padx=5, pady=1, sticky="nsew")
        
        self.save_map_button = tk.Button(self.window, text="Save Map", font=("Helvetica", 10), command=self.save_map, \
            borderwidth=3)
        self.save_map_button.grid(row=5, column=12, padx=5, pady=1, sticky="nsew")

        self.test_button = tk.Button(self.window, text="Test", font=("helvetica", 10), borderwidth=3)
        self.test_button.grid(row=6, column=12, padx=5, pady=1, sticky="nsew")
        
        #dqnn_train_status = tk.IntVar()
        #self.dqnn_train_checkbox = tk.Checkbutton(self.window, text="Train DQNN", variable=dqnn_train_status, \
        #    font=("Helvetica", 10))
        #self.dqnn_train_checkbox.grid(row=6, column=13, padx=5, pady=1, sticky="w")
        #self.pa.bat_ai.continue_training = self.dqnn_train_checkbox

        padding = tk.Label(self.window)
        padding.grid(row=7, column=0, columnspan=6)

        ### BATTLE AI STATUS ###
        self.bas_label = tk.Label(self.window, text="Battle AI Status", font=("Helvetica", 12))
        self.bas_label.grid(row=8, column=0, columnspan=6, padx=5, pady=5)

        self.battle_ai_listbox = ttk.Treeview(self.window, selectmode="none", columns=("Method", \
            "Model Output", "My HP", "Opp HP", ""))
        self.battle_ai_listbox.heading("#0", text="Move Used")
        self.battle_ai_listbox.heading("#1", text="Method")
        self.battle_ai_listbox.heading("#2", text="Model Output")
        self.battle_ai_listbox.heading("#3", text="My HP")
        self.battle_ai_listbox.heading("#4", text="Opp HP")
        self.battle_ai_listbox.heading("#5", text="")
        self.battle_ai_listbox.column(column="#0", width=180)
        self.battle_ai_listbox.column(column="#1", width=140)
        self.battle_ai_listbox.column(column="#2", width=180)
        self.battle_ai_listbox.column(column="#3", width=95)
        self.battle_ai_listbox.column(column="#4", width=95)
        self.battle_ai_listbox.column(column="#5", width=30)
        self.battle_ai_listbox.grid(row=9, column=0, columnspan=6, padx=5, pady=5, sticky="nsew")
        self.battle_history_list = []
        self.battle_ai_listbox.bind("<Double-1>", self.open_battle_history_details)
        self.bal_scroll = ttk.Scrollbar(self.window, orient="vertical", command=self.battle_ai_listbox.yview)
        self.bal_scroll.place(x=701, y=876, height=280)
        self.battle_ai_listbox.configure(yscrollcommand=self.bal_scroll.set)

        self.bc_label = tk.Label(self.window, text="Battles Completed:", font=("Helvetica", 10))
        self.bc_label.grid(row=10, column=0, padx=5, pady=1, sticky="w")
        self.battles_completed_var = tk.StringVar()
        self.battles_completed_var.set(str(self.pa.bat_ai.num_episodes_completed))
        self.bc_var_label = tk.Label(self.window, textvariable=self.battles_completed_var, font=("Helvetica", 10))
        self.bc_var_label.grid(row=10, column=1, padx=5, pady=1, sticky="w")

        self.ds_label = tk.Label(self.window, text="State Data Size:", font=("Helvetica", 10))
        self.ds_label.grid(row=11, column=0, padx=5, pady=1, sticky="w")
        self.data_size_var = tk.StringVar()
        self.data_size_var.set(str(len(self.pa.bat_ai.battle_data)))
        self.ds_var_label = tk.Label(self.window, textvariable=self.data_size_var, font=("Helvetica", 10))
        self.ds_var_label.grid(row=11, column=1, padx=5, pady=1, sticky="w")

        self.lr_label = tk.Label(self.window, text="Last Reward:", font=("Helvetica", 10))
        self.lr_label.grid(row=12, column=0, padx=5, pady=1, sticky="w")
        self.last_reward_var = tk.StringVar()
        self.last_reward_var.set(str(self.pa.bat_ai.last_reward))
        self.lr_var_label = tk.Label(self.window, textvariable=self.last_reward_var, font=("Helvetica", 10))
        self.lr_var_label.grid(row=12, column=1, padx=5, pady=1, sticky="w")

        self.lp_label = tk.Label(self.window, text="Last Prediction Values:", font=("Helvetica", 10))
        self.lp_label.grid(row=13, column=0, padx=5, pady=1, sticky="w")
        self.last_prediction_var = tk.StringVar()
        self.last_prediction_var.set(str(self.pa.bat_ai.action_predicted_rewards[0]))
        self.lp_var_label = tk.Label(self.window, textvariable=self.last_prediction_var, font=("Helvetica", 10))
        self.lp_var_label.grid(row=13, column=1, padx=5, pady=1, sticky="w")

        self.rnd_label = tk.Label(self.window, text="Randomness:", font=("Helvetica", 10))
        self.rnd_label.grid(row=14, column=0, padx=5, pady=1, sticky="w")
        self.randomness_var = tk.StringVar()
        self.randomness_var.set(str(self.pa.bat_ai.epsilon))
        self.rnd_var_label = tk.Label(self.window, textvariable=self.randomness_var, font=("Helvetica", 10))
        self.rnd_var_label.grid(row=14, column=1, padx=5, pady=1, sticky="w")

        ### MAPPER STATUS ###
        self.ms_label = tk.Label(self.window, text="Mapper Status", font=("Helvetica", 12))
        self.ms_label.grid(row=8, column=6, columnspan=6, padx=5, pady=5)

        self.mapper_listbox = ttk.Treeview(self.window, selectmode="none")
        self.mapper_listbox.heading("#0", text="Mapper History")
        self.mapper_listbox.grid(row=9, column=6, columnspan=6, padx=5, pady=5, sticky="nsew")
        self.mapper_listbox.column(column="#0", width=720)
        self.mapper_history_list = []
        # Binding function here
        self.ml_scroll = ttk.Scrollbar(self.window, orient="vertical", command=self.mapper_listbox.yview)
        self.ml_scroll.place(x=1436, y=876, height=280)
        self.mapper_listbox.configure(yscrollcommand=self.ml_scroll.set)

        self.la_label = tk.Label(self.window, text="Last action performed:", font=("Helvetica", 10))
        self.la_label.grid(row=10, column=6, padx=5, pady=1, sticky="w")
        self.last_action_var = tk.StringVar()
        self.last_action_var.set(str(self.pa.key_pressed))
        self.la_var_label = tk.Label(self.window, textvariable=self.last_action_var, font=("Helvetica", 10))
        self.la_var_label.grid(row=10, column=7, padx=5, pady=1, sticky="w")

        self.cd_label = tk.Label(self.window, text="Collision Detected:", font=("Helvetica", 10))
        self.cd_label.grid(row=11, column=6, padx=5, pady=1, sticky="w")
        self.collision_detected_var = tk.StringVar()
        self.collision_detected_var.set(str(self.pa.collision_type))
        self.cd_var_label = tk.Label(self.window, textvariable=self.collision_detected_var, font=("Helvetica", 10))
        self.cd_var_label.grid(row=11, column=7, padx=5, pady=1, sticky="w")

        self.update()
        self.window.mainloop()
    
    def update(self):
        if not (self.is_paused == True and self.initial == False):
            frame, map_grid = self.pa.run_step()
            #ret, frame = self.video.read()
            #map_grid = frame.copy()

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
                map_grid = cv2.copyMakeBorder(map_grid, padding, padding, 0, 0, cv2.BORDER_CONSTANT, (0, 0, 0))
            elif height > width:
                padding = int((height - width) / 2)
                map_grid = cv2.copyMakeBorder(map_grid, 0, 0, padding, padding, cv2.BORDER_CONSTANT, (0, 0, 0))

            map_grid = cv2.resize(map_grid, (720,720), interpolation=cv2.INTER_NEAREST)
            self.cur_map_grid = map_grid.copy()
            map_grid = cv2.cvtColor(map_grid, cv2.COLOR_BGR2RGBA)
            map_grid = Image.fromarray(map_grid)
            map_grid = ImageTk.PhotoImage(image=map_grid)
            self.map_frame.imgtk = map_grid
            self.map_frame.configure(image=map_grid)

            self.battles_completed_var.set(str(self.pa.bat_ai.num_episodes_completed))
            self.data_size_var.set(str(len(self.pa.bat_ai.battle_data)))
            self.last_reward_var.set(str(self.pa.bat_ai.last_reward))
            print_prediction = []
            for i in self.pa.bat_ai.action_predicted_rewards[0]:
                print_prediction.append("{:.1f}".format(i))
            self.last_prediction_var.set(str(print_prediction))
            self.randomness_var.set(str(self.pa.bat_ai.epsilon))
            self.last_action_var.set(str(self.pa.key_pressed))
            self.collision_detected_var.set(str(self.pa.collision_type))

            if len(self.battle_history_list) != len(self.pa.bat_ai.battle_history_list):
                # After this, two lists should become equal
                self.battle_history_list.append(self.pa.bat_ai.battle_history_list[-1])
                new_item = self.battle_history_list[-1]
                self.battle_ai_listbox.insert("", 0, text=self.attacks[new_item.text], \
                    values=(new_item.method_used, new_item.model_output, new_item.my_hp, \
                    new_item.enemy_hp, ""))
            
            if len(self.mapper_history_list) != len(self.pa.mapper_history_list):
                # After this the two lists should become equal
                self.mapper_history_list.append(self.pa.mapper_history_list[-1])
                new_item = self.mapper_history_list[-1]
                self.mapper_listbox.insert("", 0, text=new_item.text)

        self.initial = False
        self.window.after(1, self.update)
    
    def pause_ai(self):
        self.is_paused = not self.is_paused
        if (self.pause_button_text.get() == "Pause"):
            self.pause_button_text.set("Resume")
        else:
            self.pause_button_text.set("Pause")
    
    def save_map(self):
        temp = cv2.resize(self.cur_map_grid, (1080,1080), interpolation=cv2.INTER_NEAREST)
        cv2.imwrite("saved_maps/" + str(self.map_num) + ".png", temp)
        self.map_num += 1
    
    def open_battle_history_details(self, event):
        index = self.battle_ai_listbox.index(self.battle_ai_listbox.selection()[0])
        index = len(self.battle_history_list) - 1 - index
        temp_item = self.battle_history_list[index]
        output_str = f"Last Move Selected: {temp_item.text}" + "\n" + f"Move Selection Method: {temp_item.method_used}" + "\n" + \
            f"Last Prediction Values: {self.pa.bat_ai.action_predicted_rewards[0]}" + \
            "\n" + f"Pokemon HP: {temp_item.my_hp}" + "\n" + f"Enemy HP: {temp_item.enemy_hp}" + "\n" + \
            f"Battle Status: {temp_item.status}"
        messagebox.showinfo("History Info", output_str)
    
    def open_mapper_history_details(self, event):
        messagebox.showinfo("Mapper Info", "")

if __name__ == "__main__":
    gui(tk.Tk(), "poke.AI")