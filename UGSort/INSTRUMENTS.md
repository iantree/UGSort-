UGSORT INSTRUMENTATION
----------------------

From version v1.17 up there is an optional instrumentation package available in the testbed application. In order to activate the instrumentation
package the application must be built specifying the compile preprocessor definition INSTRUMENTED and the appropriate <instruments> section must
be added to the application configuration XML document. The instrumentation package enables three optional instruments for recording application
activity at runtime, all instruments output a CSV file containing the recorded variables at the designated reporting interval.

INSTRUMENTS
-----------

1.	Pile-Up - this instrument will record the number of keys present in each store on the store chain at each interval during execution.
	The columns of the CSV output are as follows :-
				
	"Cycle"       -  The reporting cycle number.		
	"Records"     -  The count of records (keys) that have been input to the sort.
	"Stores"      -  The count of stores currently in the store chain.
	"RecsInStore" -  The count of records (keys) in this store (one column per store).
	
2.	Insert activity - this instrument will record various activity values from the key insertion code paths.
	The columns of the CSV output are as follows :-
	
	"Cycle"       -  The reporting cycle number.		
	"Records"     -  The count of records (keys) that have been input to the sort.
	"Stores"      -  The count of stores currently in the store chain.				
	"Compares"    -  The count of key comparisons perfomed during the current reporting interval.			
	"Hi-Hits"     -  The number of keys that were added to the high key of a store during the current reporting interval.	
	"Lo-Hits"     -  The number of keys that were added to the low key of a store during the current reporting interval.
	"New-Stores"  -  The number of new stores added to the store chain during the current reporting interval.
	"PMs"         -  The number of pre-emptive merges performed during the current reporting interval.
	
3.	Merge activity - this instrument will record the activity for each store merge operation.
	The columns of the CSV output are as follows :-

	"Cycle"       -  The reporting cycle number.		
	"Records"     -  The count of records (keys) that have been input to the sort.
	"Stores"      -  The count of stores currently in the store chain.						
	"PMrgNo"      -  The number of the pre-emptive merge.				
	"MrgNo"       -  The number of the merge operation during the current merge (pre-emptive or final).		
	"Recs1"       -  The count of records (keys) in the target store of the merge operation.
	"Recs2"       -  The count of records (keys) in the second store of the merge operation.

INSTRUMENTED BUILD
------------------

For MS Visual Studio users simply select one of the available "Instrumented" build configurations, these are available for "x86 Release" and
"x64 Release". If you wish to add additional instrumented build configurations add the appropriate section to the CMakePresets.json file adding
-DINSTRUMENTED to the "CMAKE_CXX_FLAGS" entry.
For command line users run the cmake step with the "-DINSTRUMENTED" option.

INSTRUMENTS CONFIGURATION
-------------------------

The following section has been added to the config/sort section of the sample application configuration xml document (rt/config/UGSort.xml).

<!--- OPTIONAL: Instrumentation Configuration -->
      <instruments interval="10000">
          <pileup>Stats/PU250K.csv</pileup>
          <merge>Stats/M250K.csv</merge>
          <insert>Stats/I250K.csv</insert>
      </instruments>
<!--- END: Instrumentation Configuration -->

The reporting interval is specified on the "instruments" node (set to 10000 in the sample).
Each individual instrument is optional and contains the file path (relative or absolute) for the output CSV file from the instrument.


