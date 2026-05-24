#pragma once
#include <string>
#include <vector>

#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;

class Book {
public:
    Book(BookId id, AuthorId author_id, std::string title,int publication_year)
        : id_(std::move(id))
        , author_id_(std::move(author_id))
        , title_(std::move(title))
        ,publication_year_(publication_year) {
    }

    const BookId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetTitle() const noexcept {
        return title_;
    }

    const int GetYear() const  {
        return publication_year_;
    }

    const AuthorId& GetAuthor() const {
        return author_id_;
    }

private:
    BookId id_;
    AuthorId author_id_;
    std::string title_;
    int publication_year_;
};

class BookRepository {
public:
    virtual void Save(const Book& book) = 0;
    virtual std::vector<Book> GetAll() = 0;
    virtual std::vector<Book> GetByAuthor(const AuthorId& author_id) = 0;

protected:
    ~BookRepository() = default;
};

}  // namespace domain
