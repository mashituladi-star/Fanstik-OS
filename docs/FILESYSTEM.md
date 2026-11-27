# Файловая система и проводник

> Fanstik-OS ещё не имеет готовой файловой системы и GUI-проводника. Этот документ задаёт цель и критерии, не описывает уже реализованные функции.

## Файловая система
Целевая собственная filesystem — FanstikFS, если аудит/прототип подтвердит, что разработка и безопасное восстановление реалистичны. До надёжного этапа тестирования не использовать её для реальных пользовательских данных.

Требования:
- on-disk layout и versioning;
- блоки, метаданные, каталоги и файлы;
- создание, чтение, запись, append, truncate, удаление, rename, move;
- вложенные каталоги и корректные пути;
- размеры, timestamps и права/ownership по мере появления userspace;
- VFS, mount points, file descriptors и block-device layer;
- проверка границ и переполнений, повреждённых метаданных, ошибок I/O;
- повторное монтирование и persistence после reboot;
- тесты на пустые/большие файлы, каталоги, отказ, повреждение и повторный запуск;
- журналирование/восстановление, когда возможно.

EFI System Partition должна оставаться совместимой с UEFI и FAT32. Основная FS не должна нарушать UEFI boot requirements.

## Файловый менеджер
Реальный графический клиент VFS, а не hard-coded mock data:
- навигация назад/вперёд/вверх, path/address bar;
- реальные каталоги и файлы, list/grid view;
- Home, Desktop, Documents, Downloads, Pictures и другие пользовательские каталоги, создаваемые/обрабатываемые реальной системой;
- create file/folder, open, copy, move, paste, delete, rename;
- selection и multi-selection;
- search, sorting, filtering;
- properties, size, type, modified time;
- context menu и error states;
- hidden files toggle;
- keyboard navigation и mouse support;
- корректное отображение длинных имён, пустых папок, permissions и ошибок.

## Desktop integration
Проводник должен запускаться как приложение в window manager, получать file operations через системные API, не обращаться к диску в обход FS и корректно обрабатывать ошибки/недоступные mount points.

## Приёмка
Тестировать реальные операции с persistent disk в QEMU: создать файл/папку, записать данные, переименовать, переместить, удалить, reboot, убедиться в сохранении оставшихся данных. Визуальные тесты GUI проверяют отсутствие обрезания и наложений, но не заменяют filesystem tests.