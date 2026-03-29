from data_gen import Generate_Random_Number

# 9000 limbs for 1 ms


filename = 'EXTRA_data.txt'
for i in range(8500, 9000+1, 100):
    Generate_Random_Number(i, filename)
    Generate_Random_Number(i, filename)

print("1Done!")

for i in range(3_500_000, 4_000_000+1, 100_000):
    Generate_Random_Number(i, filename)
    Generate_Random_Number(i, filename)
print("2Done!")