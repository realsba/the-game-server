// file      : main.cpp
// author    : sba <bohdan.sadovyak@gmail.com>

#include "Application.hpp"

#include "version.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>

#include <spdlog/spdlog.h>

asio::io_context                        iocMain;
asio::signal_set                        sigHup{iocMain, SIGHUP};
asio::signal_set                        sigUsr1{iocMain, SIGUSR1};
asio::signal_set                        sigUsr2{iocMain, SIGUSR2};
boost::posix_time::ptime                startTime;
pid_t                                   pid;
std::unique_ptr<Application>            application;

void showStatistic()
{
  application->info();
  // TODO: implement
  // spdlog::info("Uptime: {}", boost::posix_time::microsec_clock::universal_time() - startTime);
}

// TODO: schedule operation instead of explicit run
void sigTermHandler(const boost::system::error_code& ec, int signal)
{
  if (!ec) {
    spdlog::info("Terminate");
    application->stop();
    iocMain.stop();
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

#include <boost/asio/buffer.hpp>

int main(int argc, char** argv)
{
  startTime = boost::posix_time::microsec_clock::universal_time();
  pid = getpid();

  std::setlocale(LC_ALL, "");

  spdlog::info("{} ({}) started. pid={}", APP_NAME, APP_VERSION, pid);

  try {
    application = std::make_unique<Application>("thegame.conf");

    asio::signal_set sig(iocMain, SIGINT, SIGTERM);
    sig.async_wait(&sigTermHandler);
    sigHup.async_wait(&sigHupHandler);
    sigUsr1.async_wait(&sigUsr1Handler);
    sigUsr2.async_wait(&sigUsr2Handler);

    application->start();
    iocMain.run();
    application->stop();
    showStatistic();
  } catch (const std::exception& e) {
    spdlog::error("Caught exception: {}", e.what());
  } catch (...) {
    spdlog::error("Caught an unknown exception");
  }

  application.reset();

  spdlog::info("{} ({}) stopped. pid={}", APP_NAME, APP_VERSION, pid);

  return EXIT_SUCCESS;
}
