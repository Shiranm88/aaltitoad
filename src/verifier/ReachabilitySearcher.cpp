/**
 * Copyright (C) 2020 Asger Gitz-Johansen

   This file is part of aaltitoad.

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
#include <verifier/ReachabilitySearcher.h>
#include <verifier/TTASuccessorGenerator.h>

#include <tinytimer/Timer.hpp>
#include <extensions/tree_extensions.h>
#include <verifier/trace_output/TTAResugarizer.h>

bool IsQuerySatisfiedHelper(const Query& query, const TTA& state) {
    switch (query.root.type) {
        case NodeType_t::Location: {
            auto ddd = state.GetCurrentLocationsLocationsOnly();
            bool ret = std::find(ddd.begin(), ddd.end(), TTAResugarizer::Unsugar(query.root.token)) != ddd.end();
            return ret;
        }
        case NodeType_t::Deadlock: return state.IsDeadlocked(); //// Deadlocked and is immediate. If we are not immediate, we can still tock (unless the interesting variables set is empty)
        case NodeType_t::LogicAnd: return IsQuerySatisfiedHelper(query.children[0], state) && IsQuerySatisfiedHelper(query.children[1], state);
        case NodeType_t::LogicOr:  return IsQuerySatisfiedHelper(query.children[0], state) || IsQuerySatisfiedHelper(query.children[1], state);
        case NodeType_t::CompLess:
        case NodeType_t::CompLessEq:
        case NodeType_t::CompNeq:
        case NodeType_t::CompEq:
        case NodeType_t::CompGreater:
        case NodeType_t::CompGreaterEq: {
            std::string exprstring{}; // This string can technically be precompiled.
            query.children[0].tree_apply([&exprstring]( const ASTNode& node ){ exprstring += node.token; });
            exprstring += query.root.token;
            query.children[1].tree_apply([&exprstring]( const ASTNode& node ){ exprstring += node.token; });
            spdlog::debug("Assembled expression '{0}'", exprstring);
            calculator c(exprstring.c_str());
            return c.eval(state.GetSymbols()).asBool();
        }
        case NodeType_t::SubExpr:
        case NodeType_t::Finally:
        case NodeType_t::Globally:
        case NodeType_t::Next:
        case NodeType_t::Until:
        case NodeType_t::Exists:
        case NodeType_t::Forall:    return IsQuerySatisfiedHelper(query.children[0], state);
        case NodeType_t::Negation:  return !IsQuerySatisfiedHelper(query.children[0], state);
        // These are handled elsewhere, so they should output an error
        case NodeType_t::Literal:
        case NodeType_t::Var: // yes, even boolean valued variables are required to be on the lhs of an "== true" expression.
        case NodeType_t::UNKNOWN:
        default: spdlog::error("Something went wrong evaluating the query."); break;
    }
    return false;
}

bool ReachabilitySearcher::IsQuerySatisfied(const Query& query, const TTA &state) {
    if(query.root.type == NodeType_t::Forall && query.children.begin()->root.type == NodeType_t::Globally) {
        auto invertedQ = Query(ASTNode{NodeType_t::Negation, "!"});
        invertedQ.insert(query);
        return IsQuerySatisfiedHelper(invertedQ, state);
    }
    if(query.root.type != NodeType_t::Exists) {
        spdlog::critical("Only reachability queries are supported right now, sorry.");
        return false;
    }
    return IsQuerySatisfiedHelper(query, state);
}

void ReachabilitySearcher::AreQueriesSatisfied(std::vector<QueryResultPair>& queries, const TTA& state, size_t state_hash) {
    for(auto & query : queries) {
        if(!query.answer) {
            query.answer = IsQuerySatisfied(*query.query, state);
            if (query.answer) {
                query.acceptingStateHash = state_hash;
                query.acceptingState.tta = state; // TODO: This is terrible
                auto ss = ConvertASTToString(*query.query);
                spdlog::info("Query '{0}' is satisfied!", ss);
                spdlog::debug("Query '{0}' was satisfied in state: \n{1}", ss, state.GetCurrentStateString());
            }
        }
    }
}

void ReachabilitySearcher::OutputResults(const std::vector<QueryResultPair>& results) {
    if(CLIConfig::getInstance()["output"]) {
        std::ofstream outputFile{CLIConfig::getInstance()["output"].as_string(), std::ofstream::trunc};
        for(auto& r : results) {
            outputFile << ConvertASTToString(*r.query) << " : " << std::boolalpha << r.answer << "\n";
        }
    }
}

void ReachabilitySearcher::PrintResults(const std::vector<QueryResultPair>& results) {
    OutputResults(results);
    spdlog::info("==== QUERY RESULTS ====");
    for(const auto& r : results) {
        spdlog::info("===================="); // Delimiter to make it easier to read
        spdlog::info("{0} : {1}", ConvertASTToString(*r.query), r.answer);
        auto stateHash = r.acceptingStateHash;
        auto state = r.acceptingState;
        std::vector<std::string> trace{};
        while(stateHash != 0) { // 0 indicates "no parent"
            spdlog::trace("trace hash: {0}", stateHash);
            auto exists = Passed.find(stateHash) != Passed.end();
            if(exists) {
                auto range = Passed.equal_range(stateHash);
                auto count = Passed.count(stateHash);
                if(count > 1)
                    spdlog::warn("HASH COLLISIONS: {0}", count);

                if(stateHash == range.first->second.prevStateHash) {
                    spdlog::critical("Breaking out of infinite loop. Something is wrong.");
                    break;
                }

                stateHash = range.first->second.prevStateHash;
                trace.push_back(range.first->second.tta.GetCurrentStateString());
                if(count > 1) {
                    for (auto it = range.first; it != range.second; ++it)
                        spdlog::warn(it->second.tta.GetCurrentStateString());
                }
            } else {
                spdlog::critical("Unable to resolve witnessing trace. ");
                break;
            }
        }
        spdlog::info("Trace:");
        std::reverse(trace.begin(), trace.end());
        printf("[");
        for(auto& stateStr : trace)
            printf("%s,\n", stateStr.c_str());
        printf("]\n");
    }
}

auto debug_int_as_hex_str(size_t v) {
    char buffer[64];
    sprintf(buffer, "%lx", v);
    return std::string(buffer);
}

std::string debug_get_current_state_string_human(const TTA& tta) {
    std::stringstream ss{};
    for(auto& c : tta.components)
        ss << c.second.currentLocation.identifier << ", ";
    return ss.str();
}

void debug_print_passed_list(const ReachabilitySearcher& r) {
    spdlog::trace("==== PASSED LIST ====");
    for(auto& e : r.Passed) {
        if(e.first == e.second.prevStateHash) {
            spdlog::warn("Hash:{0} Prev:{1} \tState:{2}",
                          debug_int_as_hex_str(e.first),
                          debug_int_as_hex_str(e.second.prevStateHash),
                         debug_get_current_state_string_human(e.second.tta));
            continue;
        }
        spdlog::trace("Hash:{0} Prev:{1} \tState:{2}",
                      debug_int_as_hex_str(e.first),
                      debug_int_as_hex_str(e.second.prevStateHash),
                      debug_get_current_state_string_human(e.second.tta));
    }
    spdlog::trace("====/PASSED LIST ====");
}

void debug_cleanup_waiting_list(ReachabilitySearcher& s, size_t curstatehash, const SearchState& state) {
    bool found;
    do {
        found = false;
        auto iterpair = s.Waiting.equal_range(curstatehash);
        for (auto it = iterpair.first; it != iterpair.second; ++it) {
            if (&it->second == &state) {
                s.Waiting.erase(it);
                found = true;
                break;
            }
        }
    } while(found);
}

std::string debug_get_symbol_map_string_representation(const TTA::SymbolMap& map) {
    std::ostringstream ss{};
    for(auto& symbol : map.map())
        ss << symbol.first << " :-> " << symbol.second << ", ";
    return ss.str();
}

bool ReachabilitySearcher::ForwardReachabilitySearch(const nondeterminism_strategy_t& strategy) {
    auto stateit = Waiting.begin();
    while(stateit != Waiting.end()) {
        auto& state = stateit->second;
        auto curstatehash = stateit->first;
        AreQueriesSatisfied(query_results, state.tta, curstatehash);
        if(AreQueriesAnswered(query_results)) {
            Passed.emplace(std::make_pair(curstatehash, state));
            if(!CLIConfig::getInstance()["notrace"])
                PrintResults(query_results);
            spdlog::info("Found a positive result after searching: {0} states", Passed.size());
            if(CLIConfig::getInstance()["verbosity"] && CLIConfig::getInstance()["verbosity"].as_integer() >= 6)
                debug_print_passed_list(*this);
            return true; // All the queries has been reached
        }
        // If the state is interesting, apply tock changes
        // TODO: Guards with parentheses that checks on interesting variables are not parsed properly. Expect weird behavior
        if(IsSearchStateTockable(state) && TTASuccessorGenerator::IsStateInteresting(state.tta)) {
            auto allTockStateChanges = TTASuccessorGenerator::GetNextTockStates(state.tta);
            AddToWaitingList(state.tta, allTockStateChanges, true, curstatehash);
        }
        auto allTickStateChanges = TTASuccessorGenerator::GetNextTickStates(state.tta);
        AddToWaitingList(state.tta, allTickStateChanges, false, curstatehash);

        Passed.emplace(std::make_pair(curstatehash, state));
        auto vs = debug_get_symbol_map_string_representation(state.tta.symbols);
        // spdlog::warn("New amount of states: {0}", allTickStateChanges.size());
        debug_cleanup_waiting_list(*this, curstatehash, state);
        stateit = PickStateFromWaitingList(strategy);
    }
    if(!CLIConfig::getInstance()["notrace"])
        PrintResults(query_results);
    spdlog::info("Found a negative result after searching: {0} states", Passed.size());
    if(CLIConfig::getInstance()["verbosity"].as_integer() >= 6)
        debug_print_passed_list(*this);
    return false;
}

ReachabilitySearcher::ReachabilitySearcher(const std::vector<const Query *> &queries, const TTA &initialState)
 : Passed{}, Waiting{}, query_results{}
{
    query_results.reserve(queries.size());
    auto search_state = SearchState{initialState, 0, false};
    for(auto& q : queries)
        query_results.emplace_back(false, q, 0, search_state);
    Waiting.emplace(std::make_pair(initialState.GetCurrentStateHash(), search_state));
}

void ReachabilitySearcher::AddToWaitingList(const TTA &state, const std::vector<TTA::StateChange> &statechanges, bool justTocked, size_t state_hash) {
    if(statechanges.size() <= 1) {
        for (auto& change : statechanges) {
            if(change.IsEmpty())
                continue;
            /// This is a lot of copying large data objects... Figure something out with maybe std::move
            auto nstate = state << change;
            auto nstatehash = nstate.GetCurrentStateHash();
            if (Passed.find(nstatehash) == Passed.end()) {
                if (nstatehash == state_hash)
                    spdlog::warn("Colliding state hashes!");
                Waiting.emplace(std::make_pair(nstatehash, SearchState{nstate, state_hash, justTocked}));
            }
        }
    }

    if(statechanges.size() > 1) {
        auto baseChanges = state << *statechanges.begin();
        for (auto it = statechanges.begin() + 1; it != statechanges.end(); ++it) {
            if(it->IsEmpty())
                continue;
            /// This is a lot of copying large data objects... Figure something out with maybe std::move
            auto nstate = baseChanges << *it;
            auto nstatehash = nstate.GetCurrentStateHash();
            if (Passed.find(nstatehash) == Passed.end()) {
                if (nstatehash == state_hash)
                    spdlog::warn("Colliding state hashes!");
                Waiting.emplace(std::make_pair(nstatehash, SearchState{nstate, state_hash, justTocked}));
            }
        }
    }
}

bool ReachabilitySearcher::AreQueriesAnswered(const std::vector<QueryResultPair> &qres) {
    return std::all_of(query_results.begin(), query_results.end(), [](const auto& r){ return r.answer; });
}

bool ReachabilitySearcher::IsSearchStateTockable(const SearchState& state) {
    return (!state.justTocked && !state.tta.IsCurrentStateImmediate());
}

ReachabilitySearcher::StateList::iterator ReachabilitySearcher::PickStateFromWaitingList(const nondeterminism_strategy_t& strategy) {
    if(Waiting.empty())
        return Waiting.end();
    if(Waiting.size() == 1)
        return Waiting.begin();
    switch (strategy) {
        case nondeterminism_strategy_t::PANIC:
            throw std::logic_error("Panicking on nondeterminism");
        case nondeterminism_strategy_t::VERIFICATION:
        case nondeterminism_strategy_t::PICK_FIRST:
            return Waiting.begin();
        case nondeterminism_strategy_t::PICK_LAST: {
            auto begin = Waiting.begin();
            for (auto i = 0; i < Waiting.size(); i++)
                begin++;
            return begin;
        }
        case nondeterminism_strategy_t::PICK_RANDOM:
            auto randomPick = rand() % Waiting.size();
            auto picked = Waiting.begin();
            for(int i = 0; i < randomPick; i++)
                picked++;
            return picked;
    }
    return Waiting.end();
}
