import os
import glob

snapshots = glob.glob("snapshots/*.h5")

for snapshot in snapshots:
    just_snapshot_name = os.path.basename(snapshot)
    print(just_snapshot_name)
    os.system("python keras_retinanet/bin/convert_model.py snapshots/" + just_snapshot_name + " inference_graphs/" + just_snapshot_name)

print("Conversion completed")
