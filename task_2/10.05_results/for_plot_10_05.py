import pandas as pd
import matplotlib.pyplot as plt
import os
import shutil

CSV_FILE_PATH = os.path.join("collision_results.csv")
PLOT_FILE_PATH = os.path.join("collision_plot.png")

# Ensure the results directory exists
# if not os.path.exists(RESULTS_DIR):
#    os.makedirs(RESULTS_DIR)

# Read the collision results from the CSV file
try:
    df = pd.read_csv(CSV_FILE_PATH)
except FileNotFoundError:
    print(f"Error: {CSV_FILE_PATH} not found. Please ensure the C++ program has been run and generated the CSV file.")
    exit()

# Plotting the collision data
plt.figure(figsize=(12, 8))

for column in df.columns[1:]:
    plt.plot(df["NumStrings"], df[column], marker="o", linestyle="-", label=column)

plt.title("Hash Function Collision Analysis")
plt.xlabel("Number of Strings Hashed")
plt.ylabel("Number of Collisions")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig(PLOT_FILE_PATH)
plt.show()

print(f"Collision plot saved to {PLOT_FILE_PATH}")
