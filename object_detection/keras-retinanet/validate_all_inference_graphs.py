import os
import glob

inference_graphs = glob.glob("inference_graphs/resnet50_800p_29+/*.h5")

for graph in inference_graphs:
    just_snapshot_name = os.path.basename(graph)
    print(just_snapshot_name)
    os.system("python keras_retinanet/bin/evaluate.py --backbone=resnet50 --score-threshold=0.95 csv ../data_processing/csvs/keras_validation.csv ../data_processing/csvs/labels.csv inference_graphs/resnet50_800p_29+/" + just_snapshot_name)

print("Validations completed")
