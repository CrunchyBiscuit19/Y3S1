def Sigma0(n, f):
    def visit(i):
        if i == 0:
            return f(0)
        else:
            return f(i) + visit(i - 1)
    if n < 0:
        return 0
    else:
        return visit(n)


def Sigma0_tail(n, f):
    # f(0) + f(3) + f(2) + f(1)
    def visit(i, a):
        if i == 0:
            return a
        else:
            return visit(i - 1, a + f(i))
    if n < 0:
        return 0
    else:
        return visit(n, f(0))


one = lambda a : 1
identity = lambda a : a
square = lambda a : a * a

def reference_one(n):
    if (n < 0):
        return 0
    return n + 1

def reference_identity(n):
    if (n < 0):
        return 0
    return (n * (n + 1)) / 2

def reference_square(n):
    if (n < 0):
        return 0
    return (n * (n + 1) * (2 * n + 1)) / 6

cases = [("one", one, reference_one),
         ("identity", identity, reference_identity),
         ("square", square, reference_square)]

def test_Sigma(func):
    for n in range(-5, 10 + 1):
        for name, f, reference in cases:
            expected = reference(n)
            actual = func(n, f)
            if actual != expected:
                print(f"{name}: n = {n}, expected {expected}, actual {actual}")
                return False
    return True

def test_Sigmas_same(func1, func2):
    for n in range(-5, 10 + 1):
        actual1 = func1(n, square)
        actual2 = func2(n, square)
        if actual1 != actual2:
            print(f"square: n = {n}, expected {actual1}, actual {actual2}")
            return False
    return True

print("Now testing Sigma0")
print(test_Sigma(Sigma0))
print("Now testing Sigma0_tail")
print(test_Sigma(Sigma0_tail))
print("Now testing both same outputs")
print(test_Sigmas_same(Sigma0, Sigma0_tail))
