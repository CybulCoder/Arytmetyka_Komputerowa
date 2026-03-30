from data_gen import Generate_Random_Number



filename = 'EXTRA_data.txt'
for i in range(8700, 9000+1, 50):
    Generate_Random_Number(i, filename)
    Generate_Random_Number(i, filename)

print("1Done!")

for i in range(3_650_000, 3_700_000+1, 10_000):
    Generate_Random_Number(i, filename)
    Generate_Random_Number(i, filename)
print("2Done!")