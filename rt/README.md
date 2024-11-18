# Running UGSort

Firstly edit the required parameters in the rt/Config/UGSort.xml file.

The following describes the different settings in the XML schema:
NOTE: relative file paths are all relative to the rt directory.

		Configuration XML
		-----------------

		<sort inmem="true|false" ondisk="true|false" pm="enable|disable" maxsplitters="s" maxinc="i"
			maxrecl="l>
																				This section contains the parameters that control the sort								
			inmem and ondisk are mutually exclusive. If neither is specified then sort input file size and
			key length will determine which is in effect.
			preemptive merging (pm) is enabled by default so pm="disable" will disable it
			where l is the maximum record length (default: 16kB)

			<sortin>i</sortin>
				Specifies the sort input
				where i is the relative or absolute file name of the sort input

			<sortout>o</sortout>		

				Specifies the sort output
				where o is the relative or absolute file name of the sort output

			<sortkey offset="o" length="l" ascending="true|false" descending="true|false" stable="true|false">
			</sortkey>

				Specifies the sort key - Optional
				where o is the offset in the record to the sort key
				where l is the length of the sort key
				Ascending is the default so setting ascending="false" or descending="true" will select
				descending sort order
				where stable="true" causes the input record sequence to be preserved for identical keys

		</sort>	

Settings in the configuration xml file can be overridden on the command line as follows:

		Command Line
		------------

		UGSort [<in> <out>] [<switches>]

		<in> is the sort input (sortin) file name (relative or absolute)
		<out> is the sort output (sortout) file name (relative or absolute) 
		NOTE: Both or neither may be specified here

		switches

			-pm			Enables preemptive merging
			-nopm			Disables preemptive merging
			-maxrecl:l		Specifies the maximum record length (default: 16kB)
			-inmem			Use in-memory sorting model
			-ondisk			Use on-disk sorting model
			-skoffset:o		Specifies the offset in the records to the sort key
			-sklen:l		Specifies the length of the sort key
			-ska			Specifies that the sort sequence is ascending
			-skd			Specifies that the sort sequence is descending
			-sks			Specifies that the record sequence is preserved (stable) for identical keys

Output logs are written to the rt/Logs directory.


