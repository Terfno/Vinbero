import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
sns.set()

hmgtp4d_single = [5.4525952, 3.69098752, 3.19291392, 2.95174144, 2.8272230400000002, 2.8181952719101124]
hmgtp4d_multi = [67.80092416, 33.93191936, 16.908288, 8.50132992, 4.25066496, 3.238568212134831]
endmgtp4e_single = [5.24288, 3.1981568, 3.44981504, 3.67263744, 3.78667008, 3.624067164044944]
endmgtp4e_multi = [4.94927872, 2.4641536, 1.23731968, 0.61865984, 0.30932992, 0.22243949303370783]

left = np.arange(len(hmgtp4d_single))
labels = [64, 128, 256, 512, 1024, 1424]
width = 0.3

plt.title("Payload pps (single flow)")
plt.ylabel("Mpps")
plt.xlabel("byte")

plt.bar(left, hmgtp4d_single, color='r', width=width, align='center', label="H.M.GTP4.D")
plt.bar(left+width, endmgtp4e_single, color='b', width=width, align='center', label="End.M.GTP4.E")

plt.xticks(left + width/2, labels)
plt.legend(bbox_to_anchor=(1, 1), loc='upper right', borderaxespad=0, fontsize=18)

plt.show()


left = np.arange(len(hmgtp4d_multi))
plt.title("Payload pps (multi flow)")
plt.ylabel("Mpps")
plt.xlabel("byte")

plt.bar(left, hmgtp4d_multi, color='r', width=width, align='center', label="H.M.GTP4.D")
plt.bar(left+width, endmgtp4e_multi, color='b', width=width, align='center', label="End.M.GTP4.E")

plt.xticks(left + width/2, labels)
plt.legend(bbox_to_anchor=(1, 1), loc='upper right', borderaxespad=0, fontsize=18)
plt.show()
