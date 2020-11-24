/**
 * Copyright (C) 2020 Asger Gitz-Johansen

   This file is part of mave.

    mave is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    mave is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with mave.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MAVE_CTLQUERYPARSER_H
#define MAVE_CTLQUERYPARSER_H
#include "query/QueryTypes.h"
#include "CTLIR.h"
#include <rapidjson/document.h>
#include <runtime/TTA.h>

class CTLQueryParser {
public:
    static std::vector<Query> ParseQueriesFile(const std::string& filepath, const TTA& tta);

private:
    static Query ParseQuery(const rapidjson::Document::ValueType& document, const TTA& tta);
    static Quantifier ParseQuantifier(std::string full_query);
    static Condition ParseCondition(std::string full_query, const TTA& tta);
    static Condition ParseSubCondition(const std::string& subquery, const TTA& tta);
    static bool IsDocumentProperQueryDocument(const rapidjson::Document::ValueType& document);
    static bool IsElementProperQuery(const rapidjson::Document::ValueType& document);
    static CTLIR ParseParetheses(std::string::const_iterator& iterator, std::string::const_iterator end);
};


#endif //MAVE_CTLQUERYPARSER_H
