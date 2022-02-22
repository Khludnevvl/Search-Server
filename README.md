## Search-Server

---

Учебный проект, созданный для изучение контейнеров и алгоритмов STL.
Поисковик документов с учетом минус-слов (документы с этими словами  не выдаются в поиске).

### Использование:

---

Документы задаются с помощью std::string. Указываются статус и рейтинг документа(массив int) для сортировки и поиска. По умолчанию ранжирование происходит по TF-IDF. Также возможно указать иные ключи  для ранжирования (статус документа, рейтинг, функции).
В качестве ключа можно передать собственную функцию, принимающую id, статус и рейтинг документа,возвращающую bool (если true, то документ участвует в поиске).

Функция поиска возвращает первые MAX_RESULT_DOCUMENT_COUNT (по умолчанию 5) документов. Значение можно изменить в файле "search_server.h".
Для выполнения основных функции поиска и сортировки используется класс SearchServer.

Пример использования указан в файле "main.cpp".

#### Используемые функции:
* AddDocument // Добавление документа на сервер
* FindTopDocuments // Нахождение подходящих документов


### Системные требования:
---

C++17

### Планы по доработке:

---

* Реализовать поиск файлов в каталогах компьютера.
* Создать удобное GUI

---
