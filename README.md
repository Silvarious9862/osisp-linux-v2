
# BDupes project
Утилита, позволяющая найти дубликаты файлов в указанной директории

## Перед началом работы  
  
Убедитесь, что у вас установлены libmagic
```bash
sudo dnf install file-devel
```
Соберите проект с использованием **Makefile**

```make```

Добавьте страницу man в системный каталог
```bash
sudo cp bdupes.1 /usr/share/man/man1/
sudo mandb
```

Добавьте утилиту в системный каталог
```bash
sudo cp out/release/bdupes /usr/local/bin/
```

## Использование


Для использования введите команду:
```bash
bdupes [options] <path>
```

При необходимости откройте страницу помощи
```bash
man bdupes
```
