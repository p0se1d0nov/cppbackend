#pragma once

#include <string>

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual void AddBook(int publication_year, const std::string& title, const domain::AuthorId& author_id) = 0;
    virtual std::vector<domain::Author> ShowAuthors() = 0;
    virtual std::vector<domain::Book> ShowAuthorsBook(const domain::AuthorId& author_id) = 0;
    virtual std::vector<domain::Book> ShowBooks() = 0;

protected:
    ~UseCases() = default;
};

}  // namespace app
