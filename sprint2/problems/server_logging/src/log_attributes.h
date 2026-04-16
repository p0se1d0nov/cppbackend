#pragma once
#include <boost/json.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>

// Определение ключевого слова для дополнительных данных
BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData",
                            boost::json::value)