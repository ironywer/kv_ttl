---

# KV-TTL (KVStorage with TTL)
---
## Сложности

Пусть `N` — число активных ключей, `K` — кол-во записей в диапазоне:

| Метод                       | Сложность                  |
| --------------------------- | -------------------------- |
| `set()`                     | `O(log N)`                 |
| `get()`                     | `O(log N)`                 |
| `remove()`                  | `O(log N)`                 |
| `getManySorted(start, cnt)` | `O(log N + K)`             |
| `removeOneExpiredEntry()`   | амортизированно `O(log N)` |

Оверхэд (\~120 B + буфера строк) на обслуживание одной записи, плюс \~40 B per HeapNode при TTL.

---

## Быстрый старт

### Локальная сборка с CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/kvstorage_example
```

### Тесты

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### В Docker (сборка + тесты + покрытие)

```bash
docker build -t kv_ttl:dev .
docker run --rm kv_ttl:dev kvstorage_tests
```

### Получение отчетов покрытия

```bash
docker run --rm -d --name kv_cov kv_ttl:dev sleep 60
docker cp kv_cov:/opt/coverage/build/coverage-html/ ./coverage-html
docker rm kv_cov
```

---

## Содержание репозитория

```
kv_ttl/
├── include/kv_storage.h       — реализация и интерфейс
├── src/main.cpp               — минимальный пример
├── tests/tests.cpp            — кейсы на GoogleTest
├── CMakeLists.txt
├── Dockerfile                 — образ сборки и тестов
└── README.md                  — этот файл
```

---