
import time

def vector_to_int(vec):
    """Convert the vector of uint32_t to int
    most significant word must be at the start of vector"""
    result = 0
    for word in vec:
        result = (result << 32) | word
    return result

def main(size):
    """Main function to read numbers from file, multiply them and measure execution time"""
    filename = f'data_{size}.txt'
    csv_filename = 'resultsPython.csv'
    try:
        with open(filename, 'r') as f, open(csv_filename, 'a') as csv_file:
            while True:
                line1 = f.readline()
                if not line1:
                    break
                line2 = f.readline()
                words1 = list(map(int, line1.split()))
                words2 = list(map(int, line2.split()))
                number1 = vector_to_int(words1)
                number2 = vector_to_int(words2)
                #print('number1:', number1)
                #print('number2:', number2)

                start = time.perf_counter()
                result = number1 * number2
                end = time.perf_counter()
                duration = (end - start) * 1_000_000  # microseconds

                #print(f"Python mul execution time: {duration:.0f} microseconds")
                csv_file.write(f"{size},{duration:.0f}\n")
    except:
        print('Error while reading file!')



# choose size of numbers to test
SIZE = 600

main(SIZE)