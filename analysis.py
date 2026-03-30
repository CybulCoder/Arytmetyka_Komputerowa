import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from scipy.stats import zscore


def load_file(file_name):
    try:
        df = pd.read_csv(file_name, sep=",", header=None)
    except FileNotFoundError as e:
        print(f"File {file_name} not found: {e}")
        return 
    except Exception as e:
        print(f"File {file_name} cannot be loaded: {e}")
        return
    
    df.columns = ["length", "time", "unit"]
    return df

def equalize_units(df):
    multipliers = {
    "us": 1,
    "ns": 0.001,
    "ms": 1000
    }
    df["time_us"] = df["time"] * df["unit"].map(multipliers)

def remove_outliers(df, keep_z_score=False):
    df["z-score"] = np.zeros(len(df))
    lengths = [s for s in range(50, 1001, 50)]
    for size in lengths:
        df.loc[df["length"]==size,"z-score"] = zscore(df.loc[df["length"]==size, "time_us"])
    
    df_without_outliers = df.loc[df["z-score"].abs() < 3]
    if not keep_z_score:
        df_without_outliers.drop(["z-score"], axis=1, inplace=True)
    return df_without_outliers

def calculate_stats(df):
    median = df.groupby("length")["time_us"].median()
    st_dev = df.groupby("length")["time_us"].std() #sample standard deviation
    result = pd.merge(median, st_dev, left_index=True, right_index=True)
    result.columns = ["average", "st_dev"]
    return result

def plot_one_method(df_stats, title):
    plt.figure()
    plt.scatter(df_stats.index, df_stats.average)
    plt.scatter(np.array(df_stats.index), np.array(df_stats.average) + np.array(df_stats.st_dev), marker="_", color="black")
    plt.scatter(np.array(df_stats.index), np.array(df_stats.average) - np.array(df_stats.st_dev), marker="_", color="black")
    plt.vlines(df_stats.index, np.array(df_stats.average) - np.array(df_stats.st_dev),
               np.array(df_stats.average) + np.array(df_stats.st_dev), color="black", linewidth=1)
    plt.xlabel("Długość liczb [słowa 32-bitowe]")
    plt.xlim(40, 1010)
    plt.ylabel("Czas [us]")
    plt.tight_layout()
    plt.title(title)
    plt.show()

def model(x,a,n):
    return a * x ** n

def draw_regression(df_stats, title):
    l = np.array(df_stats.index)
    t = np.array(df_stats.average)

    params,_ = curve_fit(model, l, t)

    plt.figure()
    plt.scatter(l, t, label="zmierzone czasy")
    plt.plot(l, model(l, *params), label="dopasowanie")
    plt.legend()
    plt.xlim(40, 1010)
    plt.tight_layout()
    plt.xlabel("Długość liczb [słowa 32-bitowe]")
    plt.ylabel("Czas [us]")
    plt.title(f"{title}. a={params[0]}, n={params[1]}")
    plt.show()

def workflow():
    df_python = load_file("resultsPython.csv")
    df_gmp = load_file("resultsGMP.csv")

    equalize_units(df_python)
    equalize_units(df_gmp)

    df_python = remove_outliers(df_python)
    df_gmp = remove_outliers(df_gmp)

    stats_python = calculate_stats(df_python)
    stats_gmp = calculate_stats(df_gmp)

    plot_one_method(stats_python, "Czas vs długość - python")
    plot_one_method(stats_gmp, "Czas vs długość - gmp")

    plt.figure()
    plt.plot(stats_python.index, stats_python.average, label="python")
    plt.plot(stats_gmp.index, stats_gmp.average, label="gmp")
    plt.legend()
    plt.title("Porownanie")
    plt.tight_layout()
    plt.xlim(40, 1010)
    plt.xlabel("Długość liczb [słowa 32-bitowe]")
    plt.ylabel("Czas [us]")
    plt.show()

    draw_regression(stats_python, "Regresja-python")
    draw_regression(stats_gmp, "Regresja-gmp")


workflow()