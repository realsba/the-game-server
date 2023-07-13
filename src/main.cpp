// file      : main.cpp
// author    : sba <bohdan.sadovyak@gmail.com>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>

#include <log4cpp/PropertyConfigurator.hh>

#include "Application.hpp"
#include "Logger.hpp"

#include "version.hpp"
#include "util.hpp"
#include "MemoryStream.hpp"

boost::asio::io_context                 iosMain;
boost::asio::signal_set                 sigHup(iosMain, SIGHUP);
boost::asio::signal_set                 sigUsr1(iosMain, SIGUSR1);
boost::asio::signal_set                 sigUsr2(iosMain, SIGUSR2);
boost::posix_time::ptime                startTime;
pid_t                                   pid;
Application*                            application;

void showStatistic()
{
  application->info();
  LOG_INFO << "Uptime: " << (boost::posix_time::microsec_clock::universal_time() - startTime);
}

void sigTermHandler(const boost::system::error_code& ec, int signal)
{
  if (!ec) {
    LOG_INFO << "Terminate";
    application->stop();
    iosMain.stop();
  }
}

void sigHupHandler(const boost::system::error_code& ec, int signal)
{
  if (!ec) {
    LOG_INFO << "Reload";
    application->stop();
    application->start();
    sigHup.async_wait(&sigHupHandler);
  }
}

void sigUsr1Handler(const boost::system::error_code& ec, int signal)
{
  if (!ec) {
    LOG_INFO << "Save caches";
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
  startTime = boost::posix_time::microsec_clock::universal_time();
  pid = getpid();

  std::setlocale(LC_ALL, "");

  log4cpp::PropertyConfigurator::configure("log4cpp.conf");

  LOG_INFO << APP_NAME << " (" << APP_VERSION << ") started. pid=" << pid;

  try {
    application = new Application("thegame.conf");

    boost::asio::signal_set sig(iosMain, SIGINT, SIGTERM);
    sig.async_wait(&sigTermHandler);
    sigHup.async_wait(&sigHupHandler);
    sigUsr1.async_wait(&sigUsr1Handler);
    sigUsr2.async_wait(&sigUsr2Handler);

    application->start();
    iosMain.run();
    showStatistic();
    delete application;
  } catch (const std::exception& e) {
    LOG_ERROR << e.what();
  } catch (...) {
    LOG_ERROR << "other exception";
  }

  LOG_INFO << APP_NAME << " (" << APP_VERSION << ") stopped. pid=" << pid;

  return EXIT_SUCCESS;
}
