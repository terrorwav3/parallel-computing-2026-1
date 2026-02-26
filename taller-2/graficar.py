import matplotlib.pyplot as plt
import csv

# Leer el archivo CSV
generations = []
fish_pop    = []
shark_pop   = []

with open("populations.csv", "r") as file:
    reader = csv.reader(file)
    next(reader)  # saltar el encabezado
    for row in reader:
        generations.append(int(row[0]))
        fish_pop.append(int(row[1]))
        shark_pop.append(int(row[2]))

# Graficar
plt.figure(figsize=(12, 5))
plt.plot(generations, fish_pop,  color="steelblue", label="Fish")
plt.plot(generations, shark_pop, color="tomato",    label="Sharks")

plt.title("Wa-Tor: Population over time")
plt.xlabel("Generation")
plt.ylabel("Population")
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("populations.png")
plt.show()

print("Graph saved as populations.png")