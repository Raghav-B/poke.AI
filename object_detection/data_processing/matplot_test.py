import matplotlib.pyplot as plt
import numpy as np

test = np.array([['#', '#', '#'],\
['.', '.', '.'],\
['.', '.', 'o']], dtype='|S2')

test = test.view(np.uint8)

pokemon = np.random.rand(10, 15)
pokemon[5][7] = 0

# Display matrix
plt.matshow(test)

thing = np.array([['6', '6', '6']], dtype='|S2')
thing = thing.view(np.uint8)

plt.matshow(np.concatenate((test, thing)))

plt.matshow(pokemon)
plt.show()