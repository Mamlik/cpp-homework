from collections import defaultdict

n, m = map(int, input().split())


groups = defaultdict(lambda : 0)
for i in range(m):
    ai, bi = map(int, input().split())
    if (ai + bi) / 2 >= n/2:
        groups[(ai + bi)/2  - (n / 2)] += 1
    else:
        groups[(ai + bi)/2] += 1

total = m
ans = 0
cnt = 0
for k, v in groups.items():
    cnt += 1
    ans += v * (total - v)

if cnt == 0:
    ans = 0
print(ans // 2)