Задание 4.3: телефонная книга на бинарном дереве поиска.

Контакты хранятся в BST, после каждых 5 структурных изменений выполняется DSW-балансировка.
Файл contacts.txt теперь БИНАРНЫЙ (формат PHONEBOOK_BIN_V1), несмотря на расширение .txt.
Старый текстовый contacts.txt формата PHONEBOOK_TEXT_V1 нужно удалить перед первым запуском новой версии.

Windows/MSYS2:
  mingw32-make clean
  mingw32-make test
  mingw32-make run

Удалить старый файл данных:
  mingw32-make clean-data
