with open(r"out.txt", 'r') as fp:
    num_lines = sum(1 for line in fp if line.rstrip())
    print('Primes generated:', num_lines)  # 8
