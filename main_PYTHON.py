
import time

def vector_to_int(vec):
    """Convert the vector of uint32_t to int
    most significant word must be at the start of vector"""
    result = 0
    for word in vec:
        result = (result << 32) | word
    return result

def choose_time_unit(input_micro):
    """
    Choose the appropriate time unit based on the input i microseconds
    returns tuple of (value, unit)
    """
    if input_micro < 1:
        #nanoseconds
        return input_micro * 1000, "ns"
    elif input_micro < 1000:
        #microseconds
        return input_micro, "μs"
    elif input_micro < 1_000_000:
        #miliseconds
        return input_micro / 1000, "ms"
    else:
        #seconds
        return input_micro / 1_000_000, "s"

def main_func(input_filename, output_csv):
    """Main function to read numbers from file, multiply them and measure execution time"""
    try:
        with open(input_filename, 'r') as f, open(output_csv, 'a') as csv_file:
            while True:
                line1 = f.readline()
                if not line1:
                    break
                line2 = f.readline()
                words1 = list(map(int, line1.split()))
                words2 = list(map(int, line2.split()))
                number1 = vector_to_int(words1)
                number2 = vector_to_int(words2)

                start = time.perf_counter_ns()
                _ = number1 * number2
                end = time.perf_counter_ns()

                duration_micro = (end - start) // 1000  # microseconds

                final_duration, unit = choose_time_unit(duration_micro)


                csv_file.write(f"{len(words1)},{final_duration:.2f},{unit}\n")
                #print('Done for {} words'.format(len(words1)))
    except:
        print('Error while reading file!')



# choose size of numbers to test
SIZE = 500


in_filename = 'data_{}.txt'.format(SIZE)
out_filename = 'resultsPython.csv'


main_func(in_filename, out_filename)