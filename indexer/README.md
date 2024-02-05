# INF-1101 Search Engine Application



## Compiling
In the supplied `Makefile` there are two targets: `assert_index` and `indexer`.
To be able to compile the program, you need to supply a a file for the `INDEX_SRC`, where the functions in `index.h` are defined. Otherwise the index functions will be undefined and result in a linker error.

The `Makefile` is designed to work with the current folder structure. Simply add any `.c` files you would like to include in the `src/` directory and add these to the `INDEXER_SRC` or `ASSERT_SRC` variables respectively.  Add any `.h` files to the `ìnclude/` directory.

To finish the exam, you only need to complete the `index.c` file, in accordance with the interface spesified in `index.h`

### assert_index
This is a test program to check that the index is implemented correctly.
Start by using this to test your index implementation.


### indexer
Once the index is implemented, run your tests on some real data using the `indexer` application. It takes a single argument, which is a path to the files you want to index.
In the precode there is a number of files supplied `data/cacm/` which is a good starting point. You would run t he indexer as follows:

`$ ./indexer data/cacm/`

Once the indexer has indexed all files, it will start a web server that you can interact with in your browser at [127.0.0.1:8080](http://127.0.0.1:8080) or [localhost:8080](http://localhost:8080). From here you can input your search queries.


## Wiki articles
We recommend that you try to test with more data than is available in the cacm folder. Although you will need to download this data (as it is too much to include in the precode). A nice medium sized dataset would be the Norwegian Wikipedia html backup from November 2006. It can be downloaded from here:

https://dumps.wikimedia.org/other/static_html_dumps/November_2006/no/wikipedia-no-html.7z

Which will give you a `.7z` archive file. Extract the archive using any archive software that is compatible with 7zip. The folder structure of the articles is quite elaborate, and many names contain non-ascii characters. To draw a selection from the articles and clean up the file names you can use the supplied `parse_wiki_articles.py` script. Simply run the script with 3 arguments: the input directory, the output directory, the number of articles. For instance:

`$ python parse_wiki_articles.py wiki_articles/ data/wiki/ 2000`


This will extract 2000 .html pages from the wiki. And then you can run your indexer with:

`$ ./indexer data/wiki/`

You are free to use any data you would like to test your search engine, but be sure to mention what data you used to test your application in the report.


## Profiling

It is recommended to profile your code rather than use `gettime()` directly in your code, simply because it is a lot less tedious.
To profile your code, you need to add the `-pg` flag in the `Makefile`.
When you run your application, it should generate a `gmon.out` file, which can be parsed using `gprof`.
To do this, run the following command:

`$ gprof indexer gmon.out > p.prof`

And if we examine the `p.prof` file, you should see a profile of your program:

`$ head p.prof`

```
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  ms/call  ms/call  name    
 28.14      0.09     0.09   798504     0.00     0.00  map_haskey
 14.07      0.14     0.05   399252     0.00     0.00  insert
  7.82      0.16     0.03  3084156     0.00     0.00  skew
  6.25      0.18     0.02  2055059     0.00     0.00  hash_string
  6.25      0.20     0.02   685680     0.00     0.00  map_put

```
This gives you valuable information about the function calls in your application like how many times they are called, how long each call is and the cumulative time spent in the function.

`gprof` might not be available on MacOS and Windows, but there are alternatives. The best bet is probably the [`gperftools`](https://github.com/gperftools/gperftools).

If you prefer a more GUI oriented way of profiling, **Instruments**' *Time Profiler* or **Visual Studio**'s built-in profiler should fullfill the task.

## Good Luck!