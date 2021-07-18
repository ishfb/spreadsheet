# spreadsheet

## Как проводить измерения

```bash
$ for i in st mt mtb; do echo $i; time cmake-build-release/spreadsheet $i ~/Downloads/input.txt /dev/null; done
st

real	0m27,982s
user	0m2,852s
sys	0m0,210s
mt
Thread count is 12

real	0m4,290s
user	0m3,084s
sys	0m0,208s
```

## Текущий статус

MultiThreadTwoBatches работает за то же время, что и MultiThreadOne. При этом в его профиле я не вижу, чтобы треды страдали от contention - 72% времени они занимаются подсчётом значения вершины.  