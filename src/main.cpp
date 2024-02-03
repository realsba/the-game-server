// file      : src/main.cpp
// author    : sba <bohdan.sadovyak@gmail.com>

#include "Application.hpp"

#include "version.hpp"

#include <boost/asio.hpp>

#include <fmt/chrono.h>
#include <spdlog/spdlog.h>
#include "spdlog/cfg/env.h"
#include "spdlog/cfg/argv.h"

asio::io_context                        ioContext;
asio::signal_set                        sigHup {ioContext, SIGHUP};
asio::signal_set                        sigUsr1 {ioContext, SIGUSR1};
asio::signal_set                        sigUsr2 {ioContext, SIGUSR2};
auto                                    startTime {std::chrono::steady_clock::now()};
std::unique_ptr<Application>            application;

void showStatistic()
{
  application->info();
  auto endTime = std::chrono::steady_clock::now();
  spdlog::info("Uptime: {:%H:%M:%S}", endTime - startTime);
}

void sigTermHandler(const boost::system::error_code& ec, int signal)
{
  if (!ec) {
    spdlog::info("Terminate");
    application->stop();
    ioContext.stop();
  }
}

void sigHupHandler(const boost::system::error_code& ec, int signal)
{
  if (!ec) {
    spdlog::info("Reload");
    application->stop();
    application->start();
    sigHup.async_wait(&sigHupHandler);
  }
}

void sigUsr1Handler(const boost::system::error_code& ec, int signal)
{
  if (!ec) {
    spdlog::info("Save caches");
    application->save();
    sigUsr1.async_wait(&sigUsr1Handler);
  }
}

void sigUsr2Handler(const boost::system::error_code& ec, int signal)
{
  if (!ec) {
    showStatistic();
    sigUsr2.async_wait(&sigUsr2Handler);
  }
}

int main(int argc, char** argv)
{
  std::setlocale(LC_ALL, "");

  spdlog::cfg::load_env_levels();
  spdlog::cfg::load_argv_levels(argc, argv);
  spdlog::set_pattern("%Y-%m-%d %T.%f|%^%l%$|%t|%v");

  spdlog::info("{} ({}) started", APP_NAME, APP_VERSION);

  try {
    application = std::make_unique<Application>("thegame.toml");

    asio::signal_set sig(ioContext, SIGINT, SIGTERM);
    sig.async_wait(&sigTermHandler);
    sigHup.async_wait(&sigHupHandler);
    sigUsr1.async_wait(&sigUsr1Handler);
    sigUsr2.async_wait(&sigUsr2Handler);

    application->start();
    ioContext.run();
  } catch (const std::exception& e) {
    spdlog::error("Caught exception: {}", e.what());
  } catch (...) {
    spdlog::error("Caught an unknown exception");
  }

  showStatistic();

  application.reset();

  spdlog::info("{} ({}) stopped", APP_NAME, APP_VERSION);

  return EXIT_SUCCESS;
}
