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
#include "TTA.h"
#include <extensions/overload>

TTASymbolType TTASymbolValueFromTypeAndValueStrings(const std::string& typestr, const std::string& valuestr) {
    return PopulateValueFromString(TTASymbolTypeFromString(typestr), valuestr);
}

TTASymbolType TTASymbolTypeFromString(const std::string& typestr) {
    TTASymbolType value;
    if     (typestr == "int") value = (int)0;
    else if(typestr == "float") value = (float)0;
    else if(typestr == "bool") value = (bool)false;
    else if(typestr == "string") value = (std::string)"";
    else spdlog::error("Variable type '{0}' is not supported", typestr);
    // Value string
    return value;
}

TTASymbolType PopulateValueFromString(const TTASymbolType& type, const std::string& valuestr) {
    TTASymbolType value = type;
    std::visit(overload(
            [&valuestr, &value](const float&      ){ value = std::stof(valuestr); },
            [&valuestr, &value](const int&        ){ value = std::stoi(valuestr); },
            [&valuestr, &value](const bool&       ){
                if(valuestr == "false") value = false;
                else if(valuestr == "true") value = true;
                else spdlog::error("Value '{0}' is not of boolean type", valuestr);
            },
            [&valuestr, &value](const std::string&){
                if(valuestr[0] == '\"' && valuestr[valuestr.size()-1] == '\"')
                    value = valuestr.substr(1,valuestr.size()-2);
                else
                    spdlog::error("Missing '\"' on string value '{0}'", valuestr);
            }
    ), value);
    return value;
}



// TODO: This can be optimized by calculating hashes on the state changes only.
std::size_t TTA::GetCurrentStateHash() const {
    std::size_t state = 0;
    for(auto& component : components)
        state == 0 ?    [&state, &component](){ state = std::hash<std::string>{}(component.second.currentLocationIdentifier);}() :
        hash_combine(state, component.first);

    for(auto& symbol : symbols.map()) {
        auto symbolHash = std::hash<std::string>{}(symbol.first);   // hash of the symbol identifier
        switch(symbol.second->type) {
            case INT: hash_combine(symbolHash, symbol.second.asInt());    // Combine with the symbol value
                break;
            case BOOL: hash_combine(symbolHash, symbol.second.asBool());  // Combine with the symbol value
                break;
            case REAL: hash_combine(symbolHash, symbol.second.asDouble());// Combine with the symbol value
                break;
            case STR: hash_combine(symbolHash, symbol.second.asString()); // Combine with the symbol value
                break;
        }
        hash_combine(state, symbolHash);                        // Combine with the overall state
    }
    return state;
}
