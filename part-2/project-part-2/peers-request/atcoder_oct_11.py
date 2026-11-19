def f(x):
    ans = 0
    for c in str(x):
        ans += int(c)
    return ans


n = int(input())
data = [1]
for i in range(1, n+1):
    ans = 0
    for j in range(0, i):
        ans += f(data[j])
    data.append(ans)

print(data[-1])