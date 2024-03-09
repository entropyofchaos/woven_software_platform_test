#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

struct Word
{
  std::string data;
  int count;

  Word (std::string data_ ) :
    data(data_),
    count(1)
  {}
  
  Word () :
    data(std::string()), 
    count(1)
  {}
};

static std::vector<Word> s_wordsArray;
static Word s_word;
static int s_totalFound;

// Variables for thread syncronization safety
static std::mutex s_mutex;
static std::condition_variable s_condVar;
static bool s_wordReady = false;

// Worker thread: consume words passed from the main thread and insert them
// in the 'word list' (s_wordsArray), while removing duplicates. Terminate when
// the word 'end' is encountered.
static void workerThread ()
{
  bool endEncountered = false;
  bool found = false;
  
  while (!endEncountered)
  {
    // Protect the word to read
    std::unique_lock<std::mutex> lock(s_mutex);

    // Do we have a new word?
    s_condVar.wait(lock, [] { return s_wordReady; });

    Word w(s_word); // Copy the word
    s_wordReady = false; // Reset the flag to inform the producer that we consumed the word

    s_condVar.notify_one(); // Wake up the producer
    lock.unlock(); // Unlock the lock so the producer can start producing while we finish processing
      
    endEncountered = w.data.compare("end") == 0;
      
    if (!endEncountered)
    {
      // Do not insert duplicate words
      for (auto &p : s_wordsArray)
      {
        if (!p.data.compare(w.data))
        {
          ++p.count;
          found = true;
          break;
        }
      }

      if (!found)
      {
        s_wordsArray.push_back(w);
      }
    }
  }
};

// Read input words from STDIN and pass them to the worker thread for
// inclusion in the word list.
// Terminate when the word 'end' has been entered.
//
static void readInputWords ()
{
  bool endEncountered = false;
  
  std::thread worker( workerThread );

  std::string linebuf;
  
  while (!endEncountered)
  {
    if (!std::getline(std::cin, linebuf)) // EOF or error?
    {
        // If EOF or an error occurs, treat this the same as recieving end to
        // handle this gracefully. However, leave EOF in the cin buffer so that
        // the program ends.
        linebuf = "end";
    }

    std::unique_lock<std::mutex> lock(s_mutex);
    
    // Pass the word to the worker thread
    s_word.data = linebuf;
    endEncountered = linebuf.compare("end") == 0;
    
    // Tell the worker thread there is a word and to wake up 
    s_wordReady = true;
    s_condVar.notify_one();

    // Wait for the worker thread to consume the word
    if (!endEncountered)
    {
        s_condVar.wait(lock, [] { return !s_wordReady; });
    }
  }

  worker.join(); // Wait for the worker to terminate
}

// Repeatedly ask the user for a word and check whether it was present in the word list
// Terminate on EOF
//
static void lookupWords ()
{
  bool found;
  std::string linebuf;
    
  for(;;)
  {
    found = false;

    std::printf( "\nEnter a word for lookup:" );

    if (!std::getline(std::cin, linebuf)) // EOF or error?
    {
      break;
    }

    // Initialize the word to search for
    Word w(linebuf);

    // Search for the word
    unsigned i;
    for ( i = 0; i < s_wordsArray.size(); ++i )
    {
      if (s_wordsArray[i].data.compare(w.data) == 0)
      {
        found = true;
        break;
      }
    }

    if (found)
    {
      std::printf( "SUCCESS: '%s' was present %d times in the initial word list\n",
                   s_wordsArray[i].data.c_str(), s_wordsArray[i].count);
      ++s_totalFound;
    }
    else
      std::printf( "'%s' was NOT found in the initial word list\n", w.data.c_str());
  }
}

// Custom comparison function to put words in alphabetical order
bool compareWords(const Word& a, const Word& b) {
    return a.data < b.data;
}
int main ()
{
  try
  {
    readInputWords();
    
    // Sort the words alphabetically
    std::sort(s_wordsArray.begin(), s_wordsArray.end(), compareWords);

    // Print the word list
    std::printf( "\n=== Word list:\n" );
    for (auto p : s_wordsArray)
      std::printf( "%s %d\n", p.data.c_str(), p.count);

    lookupWords();

    printf( "\n=== Total words found: %d\n", s_totalFound );
  }
  catch (std::exception & e)
  {
    std::printf( "error %s\n", e.what() );
  }
  
  return 0;
}