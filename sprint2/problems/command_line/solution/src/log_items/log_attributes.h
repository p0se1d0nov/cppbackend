#pragma once
#include "boost_json.h"
#include "boost_log.h"

// Определение ключевого слова для дополнительных данных
BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData",
                            boost::json::value)