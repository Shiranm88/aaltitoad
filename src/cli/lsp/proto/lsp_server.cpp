/**
 * aaltitoad - a verification engine for tick tock automata models
   Copyright (C) 2023 Asger Gitz-Johansen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "lsp_server.h"
#include "lsp.pb.h"
#include <future>
#include <spdlog/spdlog.h>
#include <grpcpp/support/status.h>
#include <grpc/support/log.h>
#include <unistd.h>

namespace aaltitoad::lsp::proto {
    LanguageServerImpl::LanguageServerImpl(int port, const std::string& semver) : running{false}, port{port}, semver{semver}, diagnostics_callback{}, notifications_callback{}, progress_callback{} {

    }

    LanguageServerImpl::~LanguageServerImpl() {
        diagnostics_callback.reset();
        notifications_callback.reset();
        progress_callback.reset();
    }

    void LanguageServerImpl::start() {
        auto ss = std::stringstream{} << "0.0.0.0:" << port;
        grpc::ServerBuilder builder;
        builder.AddListeningPort(ss.str(), grpc::InsecureServerCredentials());
        builder.RegisterService(this);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        spdlog::info("language server listening on {}", ss.str());
        server->Wait();
    }

    auto LanguageServerImpl::HandleDiff(grpc::ServerContext* server_context, const Diff* diff, Empty* result) -> grpc::Status {
        progress_start("diff for '"+diff->buffername()+"' received");
        notify_info(diff->buffername() + " opened");
        std::stringstream ss{};
        for(auto x : diff->vertexadditions())
            ss << "+v: {}" << x.jsonencoding();
        for(auto x : diff->edgeadditions())
            ss << "+e: {}" << x.jsonencoding();
        for(auto x : diff->vertexdeletions())
            ss << "-v: {}" << x.jsonencoding();
        for(auto x : diff->edgedeletions())
            ss << "-e: {}" << x.jsonencoding();
        notify_trace(ss.str());
        progress_end("done");
        return grpc::Status::OK;
    }

    auto LanguageServerImpl::GetServerInfo(grpc::ServerContext* server_context, const Empty* empty, ServerInfo* result) -> grpc::Status {
        result->set_name("aaltitoad-ls");
        result->set_language("hawk");
        result->set_semanticversion(semver);
        return grpc::Status::OK;
    }

    auto LanguageServerImpl::GetDiagnostics(grpc::ServerContext* server_context, const Empty* empty, grpc::ServerWriter<DiagnosticsList>* writer) -> grpc::Status {
        diagnostics_callback = [&writer](const DiagnosticsList& d){ writer->Write(d); };
        // NOTE: we now sleep forever, because we dont want to actually exit this call ever
        std::promise<void>().get_future().wait();
        return grpc::Status::OK;
    }

    auto LanguageServerImpl::GetNotifications(grpc::ServerContext* server_context, const Empty* empty, grpc::ServerWriter<Notification>* writer) -> grpc::Status {
        notifications_callback = [&writer](const Notification& n){ writer->Write(n); };
        // NOTE: we now sleep forever, because we dont want to actually exit this call ever
        std::promise<void>().get_future().wait();
        return grpc::Status::OK;
    }

    auto LanguageServerImpl::GetProgress(grpc::ServerContext* server_context, const Empty* empty, grpc::ServerWriter<ProgressReport>* writer) -> grpc::Status {
        progress_callback = [&writer](const ProgressReport& p){ writer->Write(p); };
        // NOTE: we now sleep forever, because we dont want to actually exit this call ever
        std::promise<void>().get_future().wait();
        return grpc::Status::OK;
    }

    void LanguageServerImpl::progress_start(const std::string& message) {
        progress(ProgressReportType::PROGRESS_BEGIN, message);
    }

    void LanguageServerImpl::progress(const std::string& message) {
        progress(ProgressReportType::PROGRESS_STATUS, message);
    }

    void LanguageServerImpl::progress_end(const std::string& message) {
        progress(ProgressReportType::PROGRESS_END, message);
    }

    void LanguageServerImpl::progress_end_fail(const std::string& message) {
        progress(ProgressReportType::PROGRESS_END_FAIL, message);
    }

    void LanguageServerImpl::progress(const ProgressReportType& type, const std::string& message) {
        if(!progress_callback.has_value())
            return;
        ProgressReport result{};
        result.set_type(type);
        result.set_message(message);
        result.set_title("aaltitoad-ls");
        result.set_token("ls-info");
        progress_callback.value()(result);
    }

    void LanguageServerImpl::notify_error(const std::string& message) {
        notify(NotificationLevel::NOTIFICATION_ERROR, message);
    }

    void LanguageServerImpl::notify_info(const std::string& message) {
        notify(NotificationLevel::NOTIFICATION_INFO, message);
    }

    void LanguageServerImpl::notify_warning(const std::string& message) {
        notify(NotificationLevel::NOTIFICATION_WARNING, message);
    }

    void LanguageServerImpl::notify_debug(const std::string& message) {
        notify(NotificationLevel::NOTIFICATION_DEBUG, message);
    }

    void LanguageServerImpl::notify_trace(const std::string& message) {
        notify(NotificationLevel::NOTIFICATION_TRACE, message);
    }

    void LanguageServerImpl::notify(const NotificationLevel& level, const std::string& message) {
        if(!notifications_callback.has_value())
            return;
        Notification result{};
        result.set_level(level);
        result.set_message(message);
        notifications_callback.value()(result);
    }
}
