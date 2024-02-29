#include <iostream>
#include <pthread.h>
#include <vector>

// Variable for the threads to determine when it's their turn to print a word
int group_total = 0;
int wait_group = -1;
// Variable to store the input sentence as split words
std::vector<std::string> words{};
// Variable to store which word is currently being analyzed by the threads
int current_word = 0;
// IDs for the threads wait groups
int ALPHA_GROUP = 0;
int NUMERIC_GROUP = 1;

/*
 *
 * Alphanumeric Printing Thread
 *
 */
void *alpha(void *arg) {
  // Retrieve the threads name from the argument
  char *name = (char *)arg;

  // While there's still words to analyze
  while (current_word < words.size()) {
    // Wait until it's this threads turn to print
    while (wait_group != ALPHA_GROUP) {
      // Wait
    }

    // Make sure we can still check the current word
    if (current_word >= words.size()) {
      break;
    }

    // Once it's our turn, check if it starts with a letter
    if (isalpha(words[current_word][0])) {
      // If it does, we can print and signal to look at next word
      std::cout << name << ": " << words[current_word] << std::endl;
      current_word++;
    }

    // Let the next thread know it's their turn
    wait_group = (wait_group + 1) % group_total;
  }

  return NULL;
}
/*
 *
 * Numeric Printing Thread
 *
 */
void *numeric(void *arg) {
  // Retrieve the threads name from the argument
  char *name = (char *)arg;

  // While there's still words to analyze
  while (current_word < words.size()) {
    // Wait until it's this threads turn to print
    while (wait_group != NUMERIC_GROUP) {
      // Wait
    }

    // Make sure we can still check the current word
    if (current_word >= words.size()) {
      break;
    }

    // Once it's our turn, check if it starts with a number
    // Note: We should check that the first character is a number, but for the
    // sake of avoiding infinite loops incase of other characters like !, #, or
    // anything else, we'll just check if it's not a letter
    if (!isalpha(words[current_word][0])) {
      // If it does, we can print and signal to look at next word
      std::cout << name << ": " << words[current_word] << std::endl;
      current_word++;
    }

    // Let the next thread know it's their turn
    wait_group = (wait_group + 1) % group_total;
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  /*
   *
   * Argument handling
   *
   */

  // Store the users input
  std::string arg{};

  // Analyze the amount of arguments given and handle them accordingly
  if (argc > 2) {
    // The argument should only be one string
    std::cout << "The program should only recieve one argument." << std::endl;
    return 1;
  } else if (argc == 2) {
    // We have one argument given
    arg = argv[1];
  } else {
    // We have to use cin for the input
    std::cout << "Please enter an argument: ";
    std::getline(std::cin, arg);
    std::cout << std::endl;
  }

  // Split the input sentence into words
  arg.push_back(' ');
  std::string current_word{};
  for (int i = 0; i < arg.length(); i++) {
    if (arg[i] == ' ' && !current_word.empty()) {
      words.push_back(current_word);
      current_word.clear();
    } else {
      current_word += arg[i];
    }
  }

  /*
   *
   * Thread Creation
   *
   */

  // Define the threads and their arguments
  pthread_t alpha_thread, numeric_thread;
  char *alpha_arg = (char *)"alpha";
  char *numeric_arg = (char *)"numeric";

  // Create the threads
  int status = pthread_create(&alpha_thread, NULL, alpha, (void *)alpha_arg);
  if (status != 0) {
    std::cout << "Error creating the alpha thread." << std::endl;
    return 1;
  }
  status = pthread_create(&numeric_thread, NULL, numeric, (void *)numeric_arg);
  if (status != 0) {
    std::cout << "Error creating the numeric thread." << std::endl;
    return 1;
  }

  // At this point, the threads are going, so let's signal the first thread to
  // start by iterating their wait group
  group_total = 2;
  wait_group = 0;

  // Finally, join the threads
  status = pthread_join(alpha_thread, NULL);
  if (status != 0) {
    std::cout << "Error joining the alpha thread." << std::endl;
    return 1;
  }
  status = pthread_join(numeric_thread, NULL);
  if (status != 0) {
    std::cout << "Error joining the numeric thread." << std::endl;
    return 1;
  }

  return 0;
}
