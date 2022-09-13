#ifndef LINGLONG_BOX_SRC_COMMANDS_H
#define LINGLONG_BOX_SRC_COMMANDS_H

namespace linglong {

void showHelpInfomation(int argc, char **argv);
void showHelpInfomationError(int argc, char **argv);
void create(int argc, char **argv);
void start(int argc, char **argv);
void kill(int argc, char **argv);
void delete_(int argc, char **argv);
void list(int argc, char **argv);
void state(int argc, char **argv);
void exec(int argc, char **argv);

} // namespace linglong

#endif
