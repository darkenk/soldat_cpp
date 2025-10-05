#pragma once
#include <memory>

class FLogFile;

void updateframe(std::shared_ptr<FLogFile>& InConsoleLogFile);
void apponidle(std::shared_ptr<FLogFile>& InConsoleLogFile);
