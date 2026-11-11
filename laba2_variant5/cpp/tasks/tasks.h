#ifndef LAB2_TASKS_H
#define LAB2_TASKS_H

#include "../str/open_hash.h"
#include <iosfwd>
#include <string>
#include <vector>

namespace lab2 {

void run_task1(std::istream& input, std::ostream& output);
void run_set(const std::string& filename, const std::string& query, std::ostream& output);
void run_task3(std::istream& input, std::ostream& output);
void run_task4(std::istream& input, std::ostream& output);
void run_task5(std::istream& input, std::ostream& output);
void run_task6(std::istream& input, std::ostream& output, ProbeMode mode);
void run_task7(std::istream& input, std::ostream& output);
void hash_experiment(std::ostream& output);
void run_task(const std::string& task, std::istream& input, std::ostream& output,
              ProbeMode mode = ProbeMode::Double);

} // namespace lab2
#endif
