import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from scipy.stats import zscore
from sklearn.metrics import r2_score, mean_absolute_error, mean_squared_error, root_mean_squared_error

#rozmiar wykresów
FIGSIZE = (10, 6)


def load_file(file_name):
    '''Funkcja łąduje zawartość pliku ze zmierzonymi czasami o podanej nazwie i o rozszerzeniu csv.
    Zwraca ramkę danych z tą zawartością i nazwanymi kolumnami. Jeśli plik nie istnieje funkcja zwraca None'''
    if not file_name.endswith(".csv"):
        print(f"Należy podać scieżkę do pliku o rozszerzeniu .csv")
        return
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

def equalize_units(df, unit='us'):
    '''Funkcja wyrównuje jednostki w ramce danych.
    Pierwszy argument to ramka danych a drugi to jednostka, ktorą może być:
    a) unit = 'ms' - milisekunda,
    b) unit = 'us' - mikrosekunda (domyślnie),
    c) unit = 'ns' - nanosekunda.
    Funkcja zwraca ramkę danych z wyrównanymi jednostkami
     '''
    match unit:
        case 'us':
            multipliers = {
            "us": 1,
            "ns": 0.001,
            "ms": 1000
            }
        case 'ms':
            multipliers = {
                'us': 1e-3,
                'ns': 1e-6,
                'ms': 1
            }
        case 'ns':
            multipliers = {
                'us': 1e3,
                'ms': 1e6,
                'ns': 1
            }
        case _:
            print("Należy podać jednostkę z listy ['ms', 'us', 'ns']")
            return
    df_out = df.copy()
    col_name = f"time_{unit}"
    df_out[col_name] = df_out["time"] * df["unit"].map(multipliers)
    return df_out
    #df["time_us"] = df["time"] * df["unit"].map(multipliers)

def remove_outliers(df, keep_z_score=False):
    '''Funkcja usuwa wartości odstajace dla każdej z grup według reguły trzech sigma.
    Argumenty to ramka danych oraz wartość logiczna dotycząca tego czy zostawić obliczone z-scores w
    ramce danych (domyślnie wartość False)'''

    if "time_us" in df.columns.to_list():
        col_name = "time_us"
    elif "time_ms" in df.columns.to_list():
        col_name = "time_ms"
    elif "time_ns" in df.columns.to_list():
        col_name = "time_ns"
    else:
        print("Nie znaleziono kolumny z ujednoliconą jednostką czasu (przy wywołaniu remove_outliers)")
        return

    df["z-score"] = np.zeros(len(df))
    lengths = [s for s in range(50, 1001, 50)]
    for size in lengths:
        df.loc[df["length"]==size,"z-score"] = zscore(df.loc[df["length"]==size, col_name])
    
    df_without_outliers = df.loc[df["z-score"].abs() < 3]
    if not keep_z_score:
        df_without_outliers.drop(["z-score"], axis=1, inplace=True)
    return df_without_outliers

def calculate_stats(df):
    '''Funkcja oblicza średnią i odchylenie standardowe dla każdej z grup liczb o zadanej długości.
    Argumentem jest ramka danych
    '''
    if "time_us" in df.columns.to_list():
        col_name = "time_us"
    elif "time_ms" in df.columns.to_list():
        col_name = "time_ms"
    elif "time_ns" in df.columns.to_list():
        col_name = "time_ns"
    else:
        print("Nie znaleziono kolumny z ujednoliconą jednostką czasu (przy wywołaniu calculate_stats)")
        return
    
    avg = df.groupby("length")[col_name].mean()
    st_dev = df.groupby("length")[col_name].std() #sample standard deviation
    result = pd.merge(avg, st_dev, left_index=True, right_index=True)
    result.columns = ["average", "st_dev"]
    return result

def plot_one_method(df_stats, title, unit='us'):
    ''' Funkcja tworzy wykres zależności czasu potrzebnego na wymnożenie dwóch liczb w zależności od ich rozmiaru.
    Kolorem niebieskim zaznaczone są średnie czasy dla liczb o danej długości. Pionowe czarne słupki oznaczają odchylenie standardowe
    dla danej grupy liczb.
    Oś pozioma przedstawia rozmiar liczb wyrażony w ilości słów 32-bitowych. Na osi pionowej jest czas.
    Argumentami funkcji są ramka danych ze średnimi oraz odchyleniami standardowymi,tytuł wykresu, który się na nim będzie wyświetlał,
    oraz jednostka w której mierzony jest czas (domyślnie 'us').'''
    if not unit in ['us', 'ms', 'ns']:
        print("Należy podać jednostkę z listy: ['us', 'ms', 'ns']")
        return None
    y_label = f"Czas [{unit}]"
    fig, ax = plt.subplots(figsize=FIGSIZE, constrained_layout=True)
    ax.scatter(df_stats.index, df_stats.average, color='blue')
    ax.scatter(np.array(df_stats.index), np.array(df_stats.average) + np.array(df_stats.st_dev), marker="_", color="black")
    ax.scatter(np.array(df_stats.index), np.array(df_stats.average) - np.array(df_stats.st_dev), marker="_", color="black")
    ax.vlines(df_stats.index, np.array(df_stats.average) - np.array(df_stats.st_dev),
               np.array(df_stats.average) + np.array(df_stats.st_dev), color="black", linewidth=1)
    ax.set_xlabel("Długość liczb [słowa 32-bitowe]")
    ax.set_xlim(40, 1010)
    ax.set_ylabel(y_label)
    fig.subplots_adjust(bottom=0.15, top=0.9)
    ax.set_title(title)
    plt.show()

def model(x,a,n):
    '''Model do dopasowania linii regresji bez transformacji. Postać modelu to:
    Czas = a * (dlugosc_liczby)^n, gdzie a,n to parametry do dopasowania'''
    return a * x ** n

def model_tr(x, a, n):
    '''Model do dopasowania linii regresji po transformacjach logarytmicznych obu osi. Jego postać to:
    log(czas) = log(a) + n * log(dlugosc_liczby), gdzie a,n to parametry do dopasowania.
    Zmienne czas i dlugosc_liczby powinny już być po transformacji'''
    return n * x + np.log(a) #f. liniowa po transformacji czasów i dlugosci

def draw_regression(df_stats, title, unit='us'):
    '''Funkcja dopasowuje model bez transformacji zmiennych, następnie tworzy wykres punktów danych i dopasowanego modelu.
    Argumenty to ramka danych, tytuł wykresu, oraz jednostka z listy['us', 'ms', 'ns'], w niej mierzony jest czas (domyślnie 'us').
    Funkcja zwraca ramkę danych z obliczonymi parametrami'''
    if not unit in ['us', 'ms', 'ns']:
        print("Należy podać jednostkę z listy: ['us', 'ms', 'ns']")
        return None
    y_label = f"Czas [{unit}]"
    
    l = np.array(df_stats.index)
    t = np.array(df_stats.average)

    params,pcov, _, mess, code = curve_fit(model, l, t, full_output=True)# dopasowac do tych wartosci potegi, parametry dopasowania + wyrazy nizszych rzedow itp. model jaki zobaczyc
    #sprawdzzic jakos dopasowania, zoabczyc czy bledow nie ma
    preds = model(l, *params)#przewidywania modelu
    r2 = r2_score(t, preds)
    perr = np.sqrt(np.diag(pcov))
    d = {"0":[title, 'No', round(params[0], 4), round(perr[0],4), round(params[1], 4),round(perr[1], 4),
              round(r2, 4), mean_squared_error(preds, t), root_mean_squared_error(preds, t), 
              mean_absolute_error(preds, t), mess, code]}
    res = pd.DataFrame.from_dict(data=d, orient='index',
                                columns=["title", "is_transformed", "a", "a_std_err", "n", "n_std_err", "r2", "mse", "rmse", "mae", "message", "code"])

    fig, ax = plt.subplots(figsize=FIGSIZE, constrained_layout = True)
    ax.scatter(l, t, label="zmierzone czasy", color="black")
    ax.plot(l, preds, label="dopasowanie")
    ax.legend()
    ax.set_xlabel("Długość liczb [słowa 32-bitowe]")
    ax.set_ylabel(y_label)
    ax.set_title(f"{title}. a={round(params[0], 3)}, n={round(params[1],3)}")
    fig.subplots_adjust(top=0.9, bottom=0.15)
    plt.show()
    return res

def draw_transformed_regression(df_stats, title, unit='us'):
    '''Funkcja dopasowuje model_tr z transformacją zmiennych, następnie tworzy wykres punktów danych i dopasowanego modelu.
    Argumenty to ramka danych, tytuł wykresu, oraz jednostka z listy['us', 'ms', 'ns'], w niej mierzony jest czas (domyślnie 'us').
    Funkcja zwraca ramkę danych z obliczonymi parametrami'''
    if not unit in ['us', 'ms', 'ns']:
        print("Należy podać jednostkę z listy: ['us', 'ms', 'ns']")
        return None
    y_label = f"log(t) orginalnie [{unit}]"
    l = np.array(df_stats.index)
    t = np.array(df_stats.average)
    l_tr = np.log(l)
    t_tr = np.log(t)

    params, pcov, _, mess, code = curve_fit(model_tr, l_tr, t_tr, full_output=True)
    #sprawdzzic jakos dopasowania, zoabczyc czy bledow nie ma
    preds = model(l, *params)#przewidywania modelu
    r2 = r2_score(t, preds)
    perr = np.sqrt(np.diag(pcov))
    d = {"0":[title, 'Yes', round(params[0], 4), round(perr[0],4), round(params[1], 4),round(perr[1], 4),
              round(r2, 4), mean_squared_error(preds, t_tr), root_mean_squared_error(preds, t_tr), 
              mean_absolute_error(preds, t_tr), mess, code]}
    res = pd.DataFrame.from_dict(data=d, orient='index',
                                columns=["title", "is_transformed", "a", "a_std_err", "n", "n_std_err", "r2", "mse", "rmse", "mae", "message", "code"])

    fig, ax = plt.subplots(figsize=FIGSIZE, constrained_layout = True)
    ax.plot(l_tr, model_tr(l_tr, *params), label="Dopasowanie", color="r")
    ax.scatter(l_tr, t_tr, color="b", label="Dane")
    ax.set_ylabel(y_label)
    ax.set_xlabel("log(dlugosc)")
    ax.set_title(rf"{title}. n={round(params[1],3)}, a={round(params[0],3)}, model: $\log (t)=\log (a)+n\cdot \log (dlugosc)$") # zobaczyc czy tu wszystko dziala te {} wszystkie
    fig.subplots_adjust(top=0.9, bottom=0.15)
    plt.show()
    return res

def model_teor(x, a):
    return a * x ** 1.465

def model_teor_tr(x, a):
    return np.log(a) + 1.465 * x

def workflow(unit='us'):
    '''Funkcja z ogólnym workflowem'''
    
    try:
        df_python = load_file("resultsPython.csv")
        df_gmp = load_file("resultsGMP.csv")
        df_imp = load_file("resultsToom.csv")

        df_python = equalize_units(df_python, unit)
        df_gmp = equalize_units(df_gmp, unit)
        df_imp = equalize_units(df_imp, unit)

        df_python = remove_outliers(df_python)
        df_gmp = remove_outliers(df_gmp)
        df_imp = remove_outliers(df_imp)

        stats_python = calculate_stats(df_python)
        stats_gmp = calculate_stats(df_gmp)
        stats_imp = calculate_stats(df_imp)
        
        plot_one_method(stats_python, "Czas vs długość - python", unit=unit)
        plot_one_method(stats_gmp, "Czas vs długość - gmp", unit=unit)
        plot_one_method(stats_imp, "Czas vs długość - ToomCook3", unit=unit)

        #porownanie 
        fig, ax = plt.subplots(figsize=FIGSIZE, constrained_layout = True)
        ax.plot(stats_python.index, stats_python.average, label="python")
        ax.plot(stats_gmp.index, stats_gmp.average, label="gmp")
        ax.plot(stats_imp.index, stats_imp.average, label="toomcook3")
        ax.legend()
        ax.set_title("Porównanie trzech rodzajów mnożeń")
        ax.set_xlim(40, 1010) #na sztywno ustalony (przynajmniej na razie)
        ax.set_xlabel("Długość liczb [słowa 32-bitowe]")

        ax.set_ylabel(f"Czas [{unit}]")
        fig.subplots_adjust(top=0.9, bottom=0.15)
        plt.show()

        r1 = draw_regression(stats_python, "Regresja-python", unit=unit)
        r2 = draw_regression(stats_gmp, "Regresja-gmp", unit=unit)
        r5 = draw_regression(stats_imp,"Regresja-toom", unit=unit)
        r3 = draw_transformed_regression(stats_python, "Regresja tr python", unit=unit)
        r4 = draw_transformed_regression(stats_gmp, "Regresja tr gmp", unit=unit)
        r6 = draw_transformed_regression(stats_imp, "Regresja tr toom", unit=unit)

        #dopasowanie modelu teoretycznego n=1,465
        #bez transformacji
        
        rzedy = []# wiersze do raportu
        for df, tyt in zip([stats_gmp, stats_python, stats_imp], ["model_teor gmp", "model_teor python", "model_teor imp"]):
            l = np.array(df.index)
            t = np.array(df.average)

            popt, pcov, _, mess, code = curve_fit(model_teor, l, t, full_output=True)
            preds = model_teor(l, *popt)
            r_2 = r2_score(t, preds)
            perr = np.sqrt(np.diag(pcov))
            fig, ax = plt.subplots(figsize=FIGSIZE, constrained_layout = True)
            ax.plot(l, model_teor(l, *popt), label="Dopasowanie", color="r")
            ax.scatter(l, t, color="b", label="Dane")
            ax.set_ylabel(f"t [{unit}]")
            ax.set_xlabel("log(dlugosc)")
            ax.set_title(rf"{tyt}. n=1.465, a={round(popt[0],3)}, model: $ t = a \cdot x ** 1,465 $") 
            fig.subplots_adjust(top=0.9, bottom=0.15)
            plt.show()
            d = {"0":[tyt, 'No', round(popt[0], 4), round(perr[0],4), "-","-",
              round(r_2, 4), mean_squared_error(preds, t), root_mean_squared_error(preds, t), mean_absolute_error(preds, t), mess, code]}
            res = pd.DataFrame.from_dict(data=d, orient='index',
                                columns=["title", "is_transformed", "a", "a_std_err", "n", "n_std_err", "r2", "mse", "rmse", "mae", "message", "code"])
            rzedy.append(res.copy())
        #z transformacją
        for df, tyt in zip([stats_gmp, stats_python, stats_imp], ["model_teor tr gmp", "model_teor tr python", "model_teor tr imp"]):
            l = np.array(df.index)
            t = np.array(df.average)
            l_tr = np.log(l)
            t_tr = np.log(t)

            popt, pcov, _, mess, code = curve_fit(model_teor_tr, l_tr, t_tr, full_output=True)
            preds = model_teor_tr(l_tr, *popt)
            r_2 = r2_score(t_tr, preds)
            perr = np.sqrt(np.diag(pcov))
            fig, ax = plt.subplots(figsize=FIGSIZE, constrained_layout = True)
            ax.plot(l_tr, model_teor_tr(l_tr, *popt), label="Dopasowanie", color="r")
            ax.scatter(l_tr, t_tr, color="b", label="Dane")
            ax.set_ylabel(f"t [{unit}]")
            ax.set_xlabel("log(dlugosc)")
            ax.set_title(rf"{tyt}. n=1.465, a={round(popt[0],3)}, model: $\log (t)=\log (a)+1,465\cdot \log (dlugosc)$") 
            fig.subplots_adjust(top=0.9, bottom=0.15)
            plt.show()
            d = {"0":[tyt, 'Yes', round(popt[0], 4), round(perr[0],4), "-","-",
              round(r_2, 4), mean_squared_error(preds, t_tr), root_mean_squared_error(preds, t_tr), mean_absolute_error(preds, t_tr), mess, code]}
            res = pd.DataFrame.from_dict(data=d, orient='index',
                                columns=["title", "is_transformed", "a", "a_std_err", "n", "n_std_err", "r2", "mse", "rmse", "mae", "message", "code"])
            rzedy.append(res.copy())
        print(rzedy)
        res_final = pd.concat(([r1, r2, r5, r3, r4, r6]+rzedy), ignore_index=True)
        res_final.to_csv("Raport.csv", index=False)
        print("r1 sie udal")
        
        #raport z czasami
        stats_python.columns = [c + "_py" for c in stats_python.columns]
        stats_gmp.columns = [c + "_gmp" for c in stats_gmp.columns]
        stats_imp.columns = [c + "_toom" for c in stats_imp.columns]

        df_raport1 = pd.concat([stats_gmp, stats_python, stats_imp], axis=1)
        df_raport1.to_csv("Raport_1.csv")
    except Exception as e:
        print(f"Błąd podczas analizy: {e}")

workflow()
