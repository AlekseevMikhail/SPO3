# SPO3
## Сборка

```bash
cmake -B cmake-build-debug
cmake --build cmake-build-debug
```

## Запуск 

### Компиляция

```bash
./cmake-build-debug/analyze <пути до файлов с кодом...> <путь до файла с результатом>
```

### Вывод графа потока управления

```bash
./cmake-build-debug/analyze -а <пути до файлов с кодом...> <путь до директории с результатом>
```

## Проверка работы
В файле `passwd.txt` нужно указать пароль для RemoteTasks

### Ассемблирование

```bash
./assemble.sh
```

### Запуск

```bash
./execute.sh
```
