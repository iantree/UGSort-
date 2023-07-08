#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Chimera.h																							*
//*   Suite:      xymorg Integration - Chimera CODEC																*
//*   Version:    2.1.0	  Build:  03																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2016 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*	Chimera.h																										*
//*																													*
//*	This header file contains the class definition for the Chimera class.											*
//* The class implements an adaptive Huffman (entropy) compression scheme, this is optionally supplemented by		*
//* Lempel-Ziv (77) repeated string compression and/or Run Length Encoding.											*
//* The Lempel-Ziv encoding can optionally be enhanced by the use of a dictionary (cache).							*
//* Extended symbols (LZ77) 16bit and 24 bit can be enabled.														*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 26/09/2016   -  Initial version																			*
//* 2.0.0 - 27/02/2018   -  Refactored for the inclusion of additional options										*
//* 2.1.0 - 27/02/2018   -  STRREF Offset handling changed															*
//*							Greedy Algorithm Defeating																*
//*							DICTREF encoding																		*
//*																													*
//*******************************************************************************************************************

//  Platform/Language Headers
#include	"../LPBHdrs.h"

//  Application Headers
#include	"../types.h"
#include	"../consts.h"
#include	"Bitstreams.h"																			//  Bit/Byte Stream classes

//  xymorg namespace
namespace xymorg {

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Chimera Class																									*
	//*                                                                                                                 *
	//*   This class provides the implementation of the Adaptive Chimera compression scheme.							*
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class Chimera {
	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Forward Declarations                                                                                          *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class AdaptiveHuffmanTree;
		class OffsetCODEC;
		class DictRefCODEC;

	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const USHORT		AlphabetSize = 263;													//  Size of the extended alphabet
		static const uint32_t	NEWSYMBOL = 256;													//  New symbol code
		static const uint32_t	RLE = 257;															//  Run Length Encoding code
		static const uint32_t	REPEATSTRING = 258;													//  Repeat String code
		static const uint32_t	DICTENTRY = 259;													//  Dictionary Entry (Repeated String)
		static const uint32_t	XSYMBOL = 260;														//  Extended Symbol (Doublet or Triplet)
		static const uint32_t	REPEAT = 261;														//  Repeat previous symbol
		static const uint32_t	EOS = 262;															//  End-Of-Stream code

		static const USHORT		DefaultWindowSize = 4096;											//  Default window size

		static const SWITCHES	LZPermitted = 0x00000001;											//  Permit Lempel-Ziv (77)
		static const SWITCHES	DICPermitted = 0x00000002;											//  Permit (LZ) dictionary
		static const SWITCHES	RLEPermitted = 0x00000004;											//  Permit Run-Length-Encoding (RLE)
		static const SWITCHES	XSPermitted = 0x00000008;											//  Permit Extended symbols (16 & 32 bit)
		static const SWITCHES	MSPermitted = 0x00000010;											//  Permit Modal Streaming
		static const SWITCHES	AllPermitted = 0x00000007;											//  All options permitted

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Structures                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//
		//   CStats		-		(De) Compression Statistics Structure
		//

		typedef struct CStats {
			size_t			BytesIn;																//  Input Bytes
			size_t			BytesOut;																//  Output Bytes
			size_t			Tokens;																	//  Number of Tokens in the compressed stream (in/out)
			size_t			ReuseTokens;															//  Modal re-use of last token
			size_t			NS1Tokens;																//  New Symbol (1) Tokens
			size_t			NS1Bits;																//  New Symbol (1) Bits emitted
			size_t			NS2Tokens;																//  New Symbol (Doublet) Tokens
			size_t			NS2Bits;																//  New Symbol (Doublet) Bits emitted
			size_t			NS3Tokens;																//  New Symbol (triplet) Tokens
			size_t			NS3Bits;																//  New Symbol (Triplet) Bits emitted
			size_t			ES1Tokens;																//  Existing Symbol (1) Tokens
			size_t			ES1Bits;																//  Existing Symbol (1) Bits emitted
			size_t			ES2Tokens;																//  Existing Symbol (Doublet) Tokens
			size_t			ES2Bits;																//  Existing Symbol (Doublet) Bits emitted
			size_t			ES3Tokens;																//  Existing Symbol (triplet) Tokens
			size_t			ES3Bits;																//  Existing Symbol (Triplet) Bits emitted
			size_t			DictTokens;																//  Dictionary reference tokens emitted
			size_t			DictBytes;																//  Dictionary reference bytes encoded
			size_t			DictBits;																//  Dictionary reference bits emitted
			size_t			StrTokens;																//  String reference tokens emitted
			size_t			StrBytes;																//  String reference bytes encoded
			size_t			StrBits;																//  String reference bits emitted
			size_t			RL8Tokens;																//  RLE 8 bit tokens emitted
			size_t			RL8Bytes;																//  RLE 8 bit bytes encoded
			size_t			RL8Bits;																//  RLE 8 bit bits emitted
			size_t			RL16Tokens;																//  RLE 16 bit tokens emitted
			size_t			RL16Bytes;																//  RLE 16 bit bytes encoded
			size_t			RL16Bits;																//  RLE 16 bit bits emitted
			size_t			RL32Tokens;																//  RLE 32 bit tokens emitted
			size_t			RL32Bytes;																//  RLE 32 bit bytes encoded
			size_t			RL32Bits;																//  RLE 32 bit bits emitted
		} CStats;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************


		//  Normal Constructor
		//
		//  Creates a new Chimera CODEC using the ostream for stats/diagnostics
		//
		//  PARAMETERS
		//
		//		std::ostream&			-		Reference to the ostream to use for stats and diagnostics
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		Chimera(std::ostream& TDStream) : os(TDStream) {

			//  Set the default configuration
			WindowSize = DefaultWindowSize;
			PermittedOptions = AllPermitted;
			StatsTrace = false;
			DebugTrace = false;
			StartTraceWindow = 0;
			EndTraceWindow = 9999999;

			//  Clear the statistics block
			memset(&Stats, 0, sizeof(CStats));

			//  Return to caller
			return;
		}

		//  Normal Constructor (with options)
		//
		//  Creates a new Chimera CODEC using the ostream for stats/diagnostics and setting the desired compressio options
		//
		//  PARAMETERS
		//
		//		SWITCHES				-		Configuration option switches
		//		std::ostream&			-		Reference to the ostream to use for stats and diagnostics
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		Chimera(SWITCHES ConfigOpts, std::ostream& TDStream) : os(TDStream) {

			//  Set the default configuration
			WindowSize = DefaultWindowSize;
			PermittedOptions = ConfigOpts;
			StatsTrace = false;
			DebugTrace = false;
			StartTraceWindow = 0;
			EndTraceWindow = 9999999;

			//  Clear the statistics block
			memset(&Stats, 0, sizeof(CStats));

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor	                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the current Chimera object, freeing any held resources
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		~Chimera() {

			if (DebugTrace) os << "INFO: The ChimeraCODEC at 0x" << this << " has been destroyed." << std::endl;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//
		//  Compression Functions
		//  =====================
		//

		//  compress
		//
		//  Compresses the content of an input ByteStream and expresses the compressed data in another ByteStream
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the input ByteStream
		//		ByteStream&		-		Reference to the output ByteStream
		//
		//  RETURNS
		//
		//		size_t			-		Number of bytes written to the output bytestream
		//
		//  NOTES
		//
		//

		size_t	compress(ByteStream& bsIn, ByteStream& bsOut) {
			int				BestOption = 0;															//  Best encoding option for the next token
			uint32_t		BestLength = 0;															//  Number of characters that the next token will encode
			int				DictEnt = 0;															//  Dictionary entry selected
			USHORT			StringOffset = 0;														//  Offset (back) to the start of a repeated string entry
			uint32_t		TrialLength = 0;														//  Length of a candidate encoding method
			int				RunFactor = 0;															//  Run Factor 8, 16 or 32 bit
			int				EncodedLength = 0;														//  Encoded length for a repeat string
			size_t			NettOffset = 0;															//  Nett (Effective) offset to a string
			uint32_t		XSCode = 0;																//  Extended Symbol Code
			const uint32_t	NSCode = (1 << 24) + NEWSYMBOL;											//  Extended code for a new symbol
			const uint32_t	RLECode = (1 << 24) + RLE;												//  Extended code for a Run Length Encoding
			const uint32_t	STRCode = (1 << 24) + REPEATSTRING;										//  Extended code for a Repeat String (LZ77) encoding
			const uint32_t	DICCode = (1 << 24) + DICTENTRY;										//  Extended code for a Dictionary entry
			const uint32_t	XSYMCode = (1 << 24) + XSYMBOL;											//  Extended code for an Extended Symbol
			const uint32_t	REPCode = (1 << 24) + REPEAT;											//  Extended code for a Repeat Symbol
			const uint32_t	EOSCode = (1 << 24) + EOS;												//  Extended code for end-of-stream signal code
			uint32_t		EncToken = 0;															//  Encoded token
			int				TokenLen = 0;															//  Encoded token length (bits)
			uint32_t		TempEnc = 0;															//  Temporary Encoded token
			int				TempLen = 0;															//  Temporary Encoded token length (bits)
			uint32_t		LastToken = 0;															//  Last token sent (for modal streaming)
			AdaptiveHuffmanTree		Encoder(AlphabetSize, WindowSize, os);							//  Adaptive huffman tree to use for encoding
			AdaptiveHuffmanTree		Excoder(AlphabetSize, WindowSize, os);							//  Adaptive huffman tree to use for encoding Extended Symbols
			OffsetCODEC		OffCoder(os);															//  Adaptive offset encoder/decoder
			DictRefCODEC	Dictionary(os);															//  Dictionary store and encode/decoder
			MSBitStream		OBS(bsOut, true);														//  Output Bit Stream

			//  Clear the statistics block
			memset(&Stats, 0, sizeof(CStats));

			//  Insert the extended symbols into the encoding tree
			Encoder.insertSymbol(NSCode, 1);													//  New Symbol code
			if (PermittedOptions & RLEPermitted) Encoder.insertSymbol(RLECode, 1);				//  Run Length Encoding code
			if (PermittedOptions & LZPermitted) Encoder.insertSymbol(STRCode, 1);				//  Repeat string (LZ77) code
			if (PermittedOptions & DICPermitted) Encoder.insertSymbol(DICCode, 1);				//  Dictionary String code
			if (PermittedOptions & XSPermitted) Encoder.insertSymbol(XSYMCode, 1);				//  Extended Symbol code
			if (PermittedOptions & MSPermitted) Encoder.insertSymbol(REPCode, 1);				//  Repeat Symbol code
			Encoder.insertSymbol(EOSCode, 1);													//  End-Of-Stream code

			//
			//  Loop processing the uncompressed input stream until it is exhausted
			//
			if (DebugTrace) os << "TRACE: Starting main compression loop (Trace window: " << StartTraceWindow << " - " << EndTraceWindow << ")." << std::endl;
			while (!bsIn.eos()) {

				//
				//  Select the best (longest) encoding option for the next chunk from the uncompressed input stream
				//
				//  Options:
				//
				//		0			-			Single Character entropy encoding (default)
				//		1			-			Dictionary string encoding
				//		2			-			Repeat String (LZ77) encoding
				//		3			-			Run Length Encoding 8 bit
				//		4			-			Run Length Encoding 16 bit
				//		5			-			Run Length Encoding 32 bit
				//		6			-			Extended Symbol Triplet encoding
				//		7			-			Extended Symbol Doublet encoding
				//

				BestOption = 0;
				BestLength = 0;

				//
				//  If enabled then attempt to use an existing dictionary string for the next chunk of input
				//

				if (PermittedOptions & DICPermitted) {
					BestLength = Dictionary.findLongestDictionaryString(bsIn, DictEnt);
					if (BestLength > 0) BestOption = 1;
				}

				//
				//  If enabled then attempt to use a string from the already encoded window in the buffer (LZ77)
				//

				if (PermittedOptions & LZPermitted) {
					TrialLength = findLongestNewString(bsIn, StringOffset);
					if (TrialLength > (BestLength + 2)) {
						BestOption = 2;
						BestLength = TrialLength;
					}
				}

				//
				//  If enabled then attempt to use a Run Length Encoding for the next chunk of input
				//

				if (PermittedOptions & RLEPermitted) {
					TrialLength = findLongestRun(bsIn, RunFactor);
					if (TrialLength > BestLength) {
						if (RunFactor == 8) BestOption = 3;
						else if (RunFactor == 16) BestOption = 4;
						else BestOption = 5;
						BestLength = TrialLength;
					}
				}

				//
				//  If no encoding option has been selected yet and extended symbol encoding is permitted then try an extended code
				//

				if (BestLength == 0 && PermittedOptions & XSPermitted) {
					BestLength = findExtendedSymbol(bsIn, Excoder, XSCode, Dictionary);
					if (BestLength == 3) BestOption = 6;
					else if (BestLength == 2) BestOption = 7;
				}

				//  
				//  Defeat of the greedy nature of the algorithm.
				//  

				if (BestLength > 0) {
					//  If we could do better by dropping the current chunk then do so
					if (canDoBetter(bsIn, BestLength, Dictionary)) {
						BestOption = 0;
						BestLength = 0;
					}
				}

				//  Adjust the length for a single character
				if (BestLength == 0) BestLength = 1;

				//
				//  Trace the selected option
				//
				if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow) {
					os << "TRACE: Best option: " << BestOption << " has been selected with length: " << BestLength << " for chunk at offset: " << bsIn.getBytesRead() << "." << std::endl;
					traceChunk(bsIn, BestLength);
				}

				//
				//  Emit the selected encoding for the next chunk to the output stream
				//

				Stats.Tokens++;

				switch (BestOption) {

					//
					//  0		-		Single character entropy encoding
					//
				case 0:

					//  Build the extended symbol code for the value
					XSCode = (1 << 24) + bsIn.peek(0);

					//  See if the code already exists in the encoding tree
					if (Encoder.hasEncoding(XSCode)) {

						//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
						if (PermittedOptions & MSPermitted) {
							if (XSCode == LastToken) {
								Encoder.hasEncoding(REPCode, EncToken, TokenLen);
								Stats.ReuseTokens++;
							}
							else {
								Encoder.hasEncoding(XSCode, EncToken, TokenLen);
								LastToken = XSCode;
							}
						}
						else Encoder.hasEncoding(XSCode, EncToken, TokenLen);

						//  Emit the encoded token
						OBS.next(EncToken, TokenLen);

						//  Tracing
						if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
							os << "TRACE: [" << Stats.Tokens << "] Emitted SYMBOL (singlet): " << EncToken << " in: " << TokenLen << " bits." << std::endl;

						//  Bookeeping
						Stats.ES1Tokens++;
						Stats.ES1Bits += TokenLen;

					}
					else {

						//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
						if (PermittedOptions & MSPermitted) {
							if (LastToken == NSCode) {
								Encoder.hasEncoding(REPCode, EncToken, TokenLen);
								Stats.ReuseTokens++;
							}
							else {
								//  Emit the new symbol token
								Encoder.hasEncoding(NSCode, EncToken, TokenLen);
								LastToken = NSCode;
							}
						}
						else Encoder.hasEncoding(NSCode, EncToken, TokenLen);

						OBS.next(EncToken, TokenLen);

						//  Emit the size of the symbol (2 bits)
						OBS.next(1, 2);

						//  Emit the symbol (8 bits)
						OBS.next(XSCode, 8);

						//  Add the symbol to the tree
						Encoder.insertSymbol(XSCode, 1);

						//  Tracing
						if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
							os << "TRACE: [" << Stats.Tokens << "] Emitted NEWSYMBOL (singlet): " << EncToken << " in: " << (TokenLen + 10) << " bits." << std::endl;

						//  Bookeeping
						Stats.NS1Tokens++;
						Stats.NS1Bits += (TokenLen + 10);
					}

					break;

					//
					//  1		-		String dictionary encoding
					//
				case 1:

					//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
					if (PermittedOptions & MSPermitted) {
						if (LastToken == DICCode) {
							Encoder.hasEncoding(REPCode, EncToken, TokenLen);
							Stats.ReuseTokens++;
						}
						else {
							//  Emit the dictionary reference token
							Encoder.hasEncoding(DICCode, EncToken, TokenLen);
							LastToken = DICCode;
						}
					}
					else Encoder.hasEncoding(DICCode, EncToken, TokenLen);

					OBS.next(EncToken, TokenLen);

					//  Emit the dictionary item ID
					Dictionary.hasEncoding(DictEnt, TempEnc, TempLen);
					OBS.next(TempEnc, TempLen);

					//  Tracing
					if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
						os << "TRACE: [" << Stats.Tokens << "] Emitted DICTREF: " << DictEnt << ", Length: " << BestLength << " in: " << (TokenLen + TempLen) << " bits." << std::endl;

					//  Bookeeping
					Stats.DictTokens++;
					Stats.DictBytes += BestLength;
					Stats.DictBits += (TokenLen + TempLen);

					break;

					//
					//  2		-		Repeated String (LZ77) encoding
					//
				case 2:

					//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
					if (PermittedOptions & MSPermitted) {
						if (LastToken == STRCode) {
							Encoder.hasEncoding(REPCode, EncToken, TokenLen);
							Stats.ReuseTokens++;
						}
						else {
							//  Emit the repeated string token
							Encoder.hasEncoding(STRCode, EncToken, TokenLen);
							LastToken = STRCode;
						}
					}
					else Encoder.hasEncoding(STRCode, EncToken, TokenLen);

					OBS.next(EncToken, TokenLen);

					//  Emit the offset - variable length encode provided by the adaptive CODEC
					OffCoder.hasEncoding(StringOffset, TempEnc, TempLen);
					OBS.next(TempEnc, TempLen);

					//  Emit the string length - variable length encodin
					//  0xxxx		-		4 bit length
					//  1xxxxxxxx   -		8 bit length

					EncodedLength = BestLength - 3;
					if (EncodedLength < 16) OBS.next(EncodedLength, 5);
					else {
						EncodedLength += 256;
						OBS.next(EncodedLength, 9);
						TokenLen += 4;																	//  To maintain bookeeping totals
					}

					//  Add the string to the dictionary
					NettOffset = bsIn.getBytesRead() - StringOffset;
					Dictionary.addToDictionary(NettOffset, BestLength);

					//  Tracing
					if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow) {
						os << "TRACE: [" << Stats.Tokens << "] Emitted STRINGREF: " << NettOffset << " (-" << StringOffset << " -> [" << TempEnc << "," << TempLen << "]";
						os << "), Length: " << BestLength << " in: " << (TokenLen + TempLen + 5) << " bits." << std::endl;
						if (PermittedOptions & DICPermitted) os << "TRACE: The string has been added to the dictionary as entry index: " << (Dictionary.getEntries() - 1) << "." << std::endl;
					}

					//  Bookeeping
					Stats.StrTokens++;
					Stats.StrBytes += BestLength;
					Stats.StrBits += (TokenLen + TempLen + 5);

					break;

					//
					//  3		-		Run Length Encoding 8 Bit
					//
				case 3:

					//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
					if (PermittedOptions & MSPermitted) {
						if (LastToken == RLECode) {
							Encoder.hasEncoding(REPCode, EncToken, TokenLen);
							Stats.ReuseTokens++;
						}
						else {
							//  Emit the Run Length Encoding (RLE) token
							Encoder.hasEncoding(RLECode, EncToken, TokenLen);
							LastToken = RLECode;
						}
					}
					else Encoder.hasEncoding(RLECode, EncToken, TokenLen);

					OBS.next(EncToken, TokenLen);

					//  Emit 8 bit conditioner 2 bits
					OBS.next(0, 2);

					//  Emit the repeat count
					OBS.next(BestLength - 1, 8);

					//  Emit the symbol
					OBS.next(bsIn.peek(0), 8);

					//  Tracing
					if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
						os << "TRACE: [" << Stats.Tokens << "] RLE8: Repeats: " << (BestLength - 1) << " in: " << (TokenLen + 18) << " bits." << std::endl;

					//  Bookeeping
					Stats.RL8Tokens++;
					Stats.RL8Bytes += BestLength;
					Stats.RL8Bits += (TokenLen + 18);

					break;

					//
					//  4		-		Run Length Encoding 16 Bit
					//
				case 4:

					//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
					if (PermittedOptions & MSPermitted) {
						if (LastToken == RLECode) {
							Encoder.hasEncoding(REPCode, EncToken, TokenLen);
							Stats.ReuseTokens++;
						}
						else {
							//  Emit the Run Length Encoding (RLE) token
							Encoder.hasEncoding(RLECode, EncToken, TokenLen);
							LastToken = RLECode;
						}
					}
					else Encoder.hasEncoding(RLECode, EncToken, TokenLen);

					OBS.next(EncToken, TokenLen);

					//  Emit 16 bit conditioner 2 bits
					OBS.next(1, 2);

					//  Emit the repeat count
					OBS.next((BestLength - 2) / 2, 8);

					//  Emit the symbols
					OBS.next(bsIn.peek(0), 8);
					OBS.next(bsIn.peek(1), 8);

					//  Tracing
					if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
						os << "TRACE: [" << Stats.Tokens << "] RLE16: Repeats: " << ((BestLength - 2) / 2) << " in: " << (TokenLen + 26) << " bits." << std::endl;

					//  Bookeeping
					Stats.RL16Tokens++;
					Stats.RL16Bytes += BestLength;
					Stats.RL16Bits += (TokenLen + 26);

					break;

					//
					//  5		-		Run Length Encoding 32 Bit
					//
				case 5:

					//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
					if (PermittedOptions & MSPermitted) {
						if (LastToken == RLECode) {
							Encoder.hasEncoding(REPCode, EncToken, TokenLen);
							Stats.ReuseTokens++;
						}
						else {
							//  Emit the Run Length Encoding (RLE) token
							Encoder.hasEncoding(RLECode, EncToken, TokenLen);
							LastToken = RLECode;
						}
					}
					else Encoder.hasEncoding(RLECode, EncToken, TokenLen);

					OBS.next(EncToken, TokenLen);

					//  Emit 32 bit conditioner 2 bits
					OBS.next(3, 2);

					//  Emit the repeat count
					OBS.next((BestLength - 4) / 4, 8);

					//  Emit the symbols
					OBS.next(bsIn.peek(0), 8);
					OBS.next(bsIn.peek(1), 8);
					OBS.next(bsIn.peek(2), 8);
					OBS.next(bsIn.peek(3), 8);

					//  Tracing
					if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
						os << "TRACE: [" << Stats.Tokens << "] RLE32: Repeats: " << ((BestLength - 4) / 4) << " in: " << (TokenLen + 42) << " bits." << std::endl;

					//  Bookeeping
					Stats.RL32Tokens++;
					Stats.RL32Bytes += BestLength;
					Stats.RL32Bits += (TokenLen + 42);

					break;

					//
					//  6		-		Extended Symbol (Triplet) encoding
					//
				case 6:

					//  See if the code already exists in the encoding tree
					if (Excoder.hasEncoding(XSCode, EncToken, TokenLen)) {

						//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
						if (PermittedOptions & MSPermitted) {
							if (LastToken == XSYMCode) {
								Encoder.hasEncoding(REPCode, TempEnc, TempLen);
								Stats.ReuseTokens++;
							}
							else {
								//  Emit the code for an Extended Symbol
								Encoder.hasEncoding(XSYMCode, TempEnc, TempLen);
								LastToken = XSYMCode;
							}
						}
						else Encoder.hasEncoding(XSYMCode, TempEnc, TempLen);

						OBS.next(TempEnc, TempLen);

						//  Emit the encoded token
						OBS.next(EncToken, TokenLen);

						//  Tracing
						if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
							os << "TRACE: [" << Stats.Tokens << "] Emitted SYMBOL (Triplet): " << EncToken << " in: " << TokenLen << " bits." << std::endl;

						//  Bookeeping
						Stats.ES3Tokens++;
						Stats.ES3Bits += (TokenLen + TempLen);

						//  Record hits for the constituent symbols
						TempEnc = (1 << 24) + ((XSCode & 0x00FF0000) >> 16);
						Encoder.hasEncoding(TempEnc, EncToken, TempLen);
						TempEnc = (1 << 24) + ((XSCode & 0x0000FF00) >> 8);
						Encoder.hasEncoding(TempEnc, EncToken, TempLen);
						TempEnc = (1 << 24) + (XSCode & 0x000000FF);
						Encoder.hasEncoding(TempEnc, EncToken, TempLen);
					}
					else {

						//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
						if (PermittedOptions & MSPermitted) {
							if (LastToken == NSCode) {
								Encoder.hasEncoding(REPCode, EncToken, TokenLen);
								Stats.ReuseTokens++;
							}
							else {
								//  Emit the new symbol token
								Encoder.hasEncoding(NSCode, EncToken, TokenLen);
								LastToken = NSCode;
							}
						}
						else Encoder.hasEncoding(NSCode, EncToken, TokenLen);

						OBS.next(EncToken, TokenLen);

						//  Emit the size of the symbol (2 bits)
						OBS.next(3, 2);

						//  Emit the symbol (24 bits)
						OBS.next(XSCode & 0x00FFFFFF, 24);

						//  Add the symbol to the tree
						Excoder.insertSymbol(XSCode, 1);

						//  Tracing
						if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
							os << "TRACE: [" << Stats.Tokens << "] Emitted NEWSYMBOL (Triplet): " << EncToken << " in: " << (TokenLen + 26) << " bits." << std::endl;

						//  Bookeeping
						Stats.NS3Tokens++;
						Stats.NS3Bits += (TempLen + TokenLen + 26);
					}

					break;

					//
					//  7		-		Extended Symbol (Doublet) encoding
					//
				case 7:

					//  See if the code already exists in the encoding tree
					if (Excoder.hasEncoding(XSCode, EncToken, TokenLen)) {

						//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
						if (PermittedOptions & MSPermitted) {
							if (LastToken == XSYMCode) {
								Encoder.hasEncoding(REPCode, TempEnc, TempLen);
								Stats.ReuseTokens++;
							}
							else {
								//  Emit the code for an Extended Symbol
								Encoder.hasEncoding(XSYMCode, TempEnc, TempLen);
								LastToken = XSYMCode;
							}
						}
						else Encoder.hasEncoding(XSYMCode, TempEnc, TempLen);

						OBS.next(TempEnc, TempLen);

						//  Emit the encoded token
						OBS.next(EncToken, TokenLen);

						//  Tracing
						if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
							os << "TRACE: [" << Stats.Tokens << "] Emitted SYMBOL (Doublet): " << EncToken << " in: " << TokenLen << " bits." << std::endl;

						//  Bookeeping
						Stats.ES2Tokens++;
						Stats.ES2Bits += (TempLen + TokenLen);

						//  Record hits for the constituent symbols
						TempEnc = (1 << 24) + ((XSCode & 0x0000FF00) >> 8);
						Encoder.hasEncoding(TempEnc, EncToken, TempLen);
						TempEnc = (1 << 24) + (XSCode & 0x000000FF);
						Encoder.hasEncoding(TempEnc, EncToken, TempLen);
					}
					else {

						//  If Modal Streaming is enabled and this Token is the same as the last then adjust the encoding
						if (PermittedOptions & MSPermitted) {
							if (LastToken == NSCode) {
								Encoder.hasEncoding(REPCode, EncToken, TokenLen);
								Stats.ReuseTokens++;
							}
							else {
								//  Emit the new symbol token
								Encoder.hasEncoding(NSCode, EncToken, TokenLen);
								LastToken = NSCode;
							}
						}
						else Encoder.hasEncoding(NSCode, EncToken, TokenLen);

						OBS.next(EncToken, TokenLen);

						//  Emit the size of the symbol (2 bits)
						OBS.next(2, 2);

						//  Emit the symbol (16 bits)
						OBS.next(XSCode & 0x0000FFFF, 16);

						//  Add the symbol to the tree
						Excoder.insertSymbol(XSCode, 1);

						//  Tracing
						if (DebugTrace && bsIn.getBytesRead() >= StartTraceWindow && bsIn.getBytesRead() <= EndTraceWindow)
							os << "TRACE: [" << Stats.Tokens << "] Emitted NEWSYMBOL (Doublet): " << EncToken << " in: " << (TokenLen + 18) << " bits." << std::endl;

						//  Bookeeping
						Stats.NS2Tokens++;
						Stats.NS2Bits += (TempLen + TokenLen + 18);
					}

					break;
				}

				//  Purge the encoded characters from the input stream
				bsIn.advance(BestLength);

				//  Update statistics
				Stats.BytesIn += BestLength;
			}

			//  Append an End-Of-Stream token to the stream
			Encoder.hasEncoding(EOSCode, EncToken, TokenLen);
			OBS.next(EncToken, TokenLen);

			//  Flush the bit stream
			OBS.flush();

			//  Update Statistics
			Stats.BytesOut = bsOut.getBytesWritten();
			Stats.Tokens++;

			//  Return to caller
			return bsOut.getBytesWritten();
		}

		//
		//  Decompression Functions
		//  =======================
		//

		//  decompress
		//
		//  Deompresses the content of an input ByteStream and expresses the decompressed data in another ByteStream
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the input ByteStream
		//		ByteStream&		-		Reference to the output ByteStream
		//
		//  RETURNS
		//
		//		size_t			-		Number of bytes written to the output bytestream
		//
		//  NOTES
		//
		//

		size_t	decompress(ByteStream& bsIn, ByteStream& bsOut) {
			const uint32_t	NSCode = (1 << 24) + NEWSYMBOL;											//  Extended code for a new symbol
			const uint32_t	RLECode = (1 << 24) + RLE;												//  Extended code for a Run Length Encoding
			const uint32_t	STRCode = (1 << 24) + REPEATSTRING;										//  Extended code for a Repeat String (LZ77) encoding
			const uint32_t	DICCode = (1 << 24) + DICTENTRY;										//  Extended code for a Dictionary entry
			const uint32_t	XSYMCode = (1 << 24) + XSYMBOL;											//  Extended code for an Extended Symbol
			const uint32_t	REPCode = (1 << 24) + REPEAT;											//  Extended code for a Repeat Symbol
			const uint32_t	EOSCode = (1 << 24) + EOS;												//  Extended code for end-of-stream signal code
			bool			EOSDetected = false;													//  End-Of-Stream signal
			BYTE*			pOutBuffer = bsOut.getBufferAddress();									//  Pointer to the start of the output buffer
			uint32_t		XSCode = 0;																//  Extended Symbol Code
			uint32_t		TempCode = 0;															//  Temporary Extended Symbol Code
			int				TempLen = 0;															//  Temporary encoding length
			uint32_t		EncToken = 0;															//  Dummy encoding token
			uint32_t		ChunkLen = 0;															//  Length of recovered chunk
			BYTE*			pChunk = nullptr;														//  Pointer to recovered chunk
			size_t			OutOffset = 0;															//  Emission offset
			USHORT			DictEntryNo = 0;														//  Dictionary entry index
			size_t			StrOffset = 0;															//  String reference relative offset
			int				EncodingUnit = 0;														//  Encoding unit for run length encoding
			BYTE			EU32[4] = {};															//  Repeated unit for run length encoding
			uint32_t		LastToken = 0;															//  Last token sent (for modal streaming)

			AdaptiveHuffmanTree		Decoder(AlphabetSize, WindowSize, os);							//  Adaptive huffman tree to use for encoding
			AdaptiveHuffmanTree		Dxcoder(AlphabetSize, WindowSize, os);							//  Adaptive huffman tree to use for encoding Extended Symbols
			OffsetCODEC				OffCoder(os);													//  Adaptive offset encoder/decoder
			DictRefCODEC			Dictionary(os);													//  Dictionary store and encode/decoder
			MSBitStream				IBS(bsIn, false);												//  Input bit stream

			//  Clear the statistics block
			memset(&Stats, 0, sizeof(CStats));

			//  Insert the extended symbols into the encoding tree
			Decoder.insertSymbol(NSCode, 1);													//  New Symbol code
			if (PermittedOptions & RLEPermitted) Decoder.insertSymbol(RLECode, 1);				//  Run Length Encoding code
			if (PermittedOptions & LZPermitted) Decoder.insertSymbol(STRCode, 1);				//  Repeat string (LZ77) code
			if (PermittedOptions & DICPermitted) Decoder.insertSymbol(DICCode, 1);				//  Dictionary String code
			if (PermittedOptions & XSPermitted) Decoder.insertSymbol(XSYMCode, 1);				//  Extended Symbol code
			if (PermittedOptions & MSPermitted) Decoder.insertSymbol(REPCode, 1);				//  Repeat previous symbol
			Decoder.insertSymbol(EOSCode, 1);													//  End-Of-Stream code

			//  Prcosess all tokens in the input stream
			if (DebugTrace) os << "INFO: Starting main decompression loop (Trace window: " << StartTraceWindow << " - " << EndTraceWindow << ")." << std::endl;
			while (!IBS.eos() && !EOSDetected) {

				//  Red the next symbol from the input stream
				XSCode = Decoder.getNextToken(IBS);

				//  If modal streaming is permitted then read the mode
				if (PermittedOptions & MSPermitted) {
					if (XSCode == REPCode) {
						XSCode = LastToken;

						//  Tracing
						if (DebugTrace && OutOffset >= StartTraceWindow && OutOffset <= EndTraceWindow) {
							os << "TRACE: [" << Stats.Tokens << "] Read REPEAT: " << XSCode << " at Offset: " << OutOffset << "." << std::endl;
						}

						Stats.ReuseTokens++;
					}
					else LastToken = XSCode;
				}
				Stats.Tokens++;

				//  If the code read was the soft End-Of-Stream then flag the state
				if (XSCode == EOSCode) EOSDetected = true;
				else {

					ChunkLen = 0;
					pChunk = bsOut.getWriteAddress();
					OutOffset = bsOut.getBytesWritten();

					//  Process the token according to the type
					switch (XSCode) {
						//
						//  New Symbol
						//

					case NSCode:
						//  Build the Extended Symbol code for the new symbol
						ChunkLen = IBS.next(2);
						XSCode = ChunkLen << 24;
						if (ChunkLen > 2) XSCode += (IBS.next(8) << 16);
						if (ChunkLen > 1) XSCode += (IBS.next(8) << 8);
						XSCode += IBS.next(8);

						//  Emit the extended symbol to the output stream
						emitSymbol(bsOut, XSCode);

						//  Add the symbol to the encoding tree
						if (ChunkLen == 1) Decoder.insertSymbol(XSCode, 1);
						else Dxcoder.insertSymbol(XSCode, 1);

						//  Tracing
						if (DebugTrace && OutOffset >= StartTraceWindow && OutOffset <= EndTraceWindow) {
							os << "TRACE: [" << Stats.Tokens << "] Read NEWSYMBOL: " << XSCode << " at Offset: " << OutOffset << "." << std::endl;
						}

						//  Bookeeping
						if (ChunkLen == 3) Stats.NS3Tokens++;
						else if (ChunkLen == 2) Stats.NS2Tokens++;
						else Stats.NS1Tokens++;

						break;

						//
						//  Dictionary String Reference
						//

					case DICCode:
						//  Process a string from the dictionary
						DictEntryNo = USHORT(Dictionary.getNextToken(IBS));

						//  Get the length of the entry
						StrOffset = Dictionary.getDictionaryString(DictEntryNo, ChunkLen);

						//  Emit the dictionary string
						for (size_t cIndex = 0; cIndex < ChunkLen; cIndex++) bsOut.next(pOutBuffer[StrOffset + cIndex]);

						//  Tracing
						if (DebugTrace && OutOffset >= StartTraceWindow && OutOffset <= EndTraceWindow) {
							os << "TRACE: [" << Stats.Tokens << "] Read DICTREF: " << DictEntryNo << " at Offset: " << OutOffset << "." << std::endl;
						}

						//  Bookeeping
						Stats.DictTokens++;
						Stats.DictBytes += ChunkLen;

						break;

						//
						//  String Reference (LZ77)
						//

					case STRCode:
						//  Get the relative offset to the source string and the string length
						StrOffset = OffCoder.getNextToken(IBS);

						ChunkLen = IBS.next(5);
						if (ChunkLen > 16) {
							ChunkLen = ChunkLen - 16;
							ChunkLen = ChunkLen << 4;
							ChunkLen += IBS.next(4);
						}
						ChunkLen += 3;

						//  Emit the string to the output stream
						for (size_t cIndex = 0; cIndex < ChunkLen; cIndex++) bsOut.next(pOutBuffer[OutOffset - StrOffset + cIndex]);

						//  Add the string to the dictionary - if enabled
						if (PermittedOptions & DICPermitted) Dictionary.addToDictionary(OutOffset, ChunkLen);

						//  Tracing
						if (DebugTrace && OutOffset >= StartTraceWindow && OutOffset <= EndTraceWindow) {
							os << "TRACE: [" << Stats.Tokens << "] Read STRINGREF offset: -" << StrOffset << ", length: " << ChunkLen << " at Offset: " << OutOffset << "." << std::endl;
							if (PermittedOptions & DICPermitted) os << "TRACE: The string has been added to the dictionary as entry index: " << (Dictionary.getEntries() - 1) << "." << std::endl;
						}

						//  Bookeeping
						Stats.StrTokens++;
						Stats.StrBytes += ChunkLen;

						break;

						//
						//  Run Length Encoding (RLE)
						//

					case RLECode:
						//  Get the encoding unit in bytes
						EncodingUnit = IBS.next(2) + 1;

						//  Get the Chunk Length (RepeatCount + 1) * EncodingUnit
						ChunkLen = (IBS.next(8) + 1) * EncodingUnit;

						//  Get the encoding repeated unit 8,16 or 32 bit
						EU32[0] = BYTE(IBS.next(8));
						if (EncodingUnit > 1) EU32[1] = BYTE(IBS.next(8));
						if (EncodingUnit > 2) {
							EU32[2] = BYTE(IBS.next(8));
							EU32[3] = BYTE(IBS.next(8));
						}

						//  Emit the run to the output stream
						for (size_t cIndex = 0; cIndex < ChunkLen; cIndex += EncodingUnit) {
							bsOut.next(EU32[0]);
							if (EncodingUnit > 1) bsOut.next(EU32[1]);
							if (EncodingUnit > 2) {
								bsOut.next(EU32[2]);
								bsOut.next(EU32[3]);
							}
						}

						//  Tracing
						if (DebugTrace && OutOffset >= StartTraceWindow && OutOffset <= EndTraceWindow) {
							os << "TRACE: [" << Stats.Tokens << "] Read RLE (" << (EncodingUnit * 8) << " bit), Repeats: " << ((ChunkLen / EncodingUnit) - 1) << " at Offset: " << OutOffset << "." << std::endl;
						}

						//  Bookeeping
						if (EncodingUnit == 1) {
							Stats.RL8Tokens++;
							Stats.RL8Bytes += ChunkLen;
						}
						else if (EncodingUnit == 2) {
							Stats.RL16Tokens++;
							Stats.RL16Bytes += ChunkLen;
						}
						else {
							Stats.RL32Tokens++;
							Stats.RL32Bytes += ChunkLen;
						}

						break;

						//
						//  Extended Symbol code (Doublet or Triplet)
						//

					case XSYMCode:

						//  Read the next code value from the auxilliary decoder
						XSCode = Dxcoder.getNextToken(IBS);

						//  Emit the symbol to the output stream
						emitSymbol(bsOut, XSCode);

						//  Get the length of the extended symbol
						ChunkLen = XSCode >> 24;

						//  Tracing
						if (DebugTrace && OutOffset >= StartTraceWindow && OutOffset <= EndTraceWindow) {
							if (ChunkLen == 3) os << "TRACE: [" << Stats.Tokens << "] Read EXTENDED SYMBOL (Triplet): " << XSCode << " at Offset: " << OutOffset << "." << std::endl;
							else os << "TRACE: [" << Stats.Tokens << "] Read EXTENDED SYMBOL (Doublet): " << XSCode << " at Offset: " << OutOffset << "." << std::endl;
						}

						//  Bookeeping
						if (ChunkLen == 3) Stats.ES3Tokens++;
						else Stats.ES2Tokens++;

						//  Record hits for the constituent symbols
						TempCode = (1 << 24) + ((XSCode & 0x0000FF00) >> 8);
						Decoder.hasEncoding(TempCode, EncToken, TempLen);
						TempCode = (1 << 24) + (XSCode & 0x000000FF);
						Decoder.hasEncoding(TempCode, EncToken, TempLen);
						if (ChunkLen == 3) {
							TempCode = (1 << 24) + ((XSCode & 0x00FF0000) >> 16);
							Decoder.hasEncoding(TempCode, EncToken, TempLen);
						}

						break;

						//
						//  Encoded Symbol (Singlet)
						//

					default:

						//  Emit the symbol to the output stream
						emitSymbol(bsOut, XSCode);

						//  Get the length of the extended symbol
						ChunkLen = XSCode >> 24;

						//  Tracing
						if (DebugTrace && OutOffset >= StartTraceWindow && OutOffset <= EndTraceWindow) {
							os << "TRACE: [" << Stats.Tokens << "] Read SYMBOL: " << XSCode << " at Offset: " << OutOffset << "." << std::endl;
						}

						//  Bookeeping
						if (ChunkLen == 3) Stats.ES3Tokens++;
						else if (ChunkLen == 2) Stats.ES2Tokens++;
						else Stats.ES1Tokens++;

						break;
					}

					//  Tracing
					if (DebugTrace && bsOut.getBytesWritten() >= StartTraceWindow && bsOut.getBytesWritten() <= EndTraceWindow) traceChunk(pChunk, ChunkLen);

					//  Bookeeping
					Stats.BytesOut += ChunkLen;
				}
			}

			//  Detect premature End-Of-Stream
			if (!EOSDetected) {
				os << "ERROR: End of the input stream has been detected before the stream is complete, the compressed stream is invalid or damaged." << std::endl;
				TempCode = Decoder.getLastEncode(TempLen);
				os << "ERROR: Last token read from the stream was: " << XSCode << ", encoded as: " << TempCode << ", length: " << TempLen << "." << std::endl;
			}

			//  Bookeeping
			Stats.BytesIn = bsIn.getBytesRead();

			//OffCoder.showStatistics();

			//  Return to caller
			return bsOut.getBytesWritten();
		}

		//
		//  Configuration Control Functions
		//  ===============================
		//

		//  setWindowSize
		//
		//  Sets the window size to use for the next compression/decompression operation. The window size MUST e identical for a paired compression and
		//  decompression.
		//
		//  PARAMETERS
		//
		//		USHORT			-		New window size to use
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void	setWindowSize(USHORT NewWindowSize) { WindowSize = NewWindowSize; return; }

		//  permitOptions
		//
		//  Sets the optional compression artefacts that are allowed in the compression stream.
		//  Any combination of the xxxPermitted constants.
		//
		//  PARAMETERS
		//
		//		SWITCHES			-		New array of options that are permitted
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void	permitOptions(SWITCHES NewOptions) { PermittedOptions = NewOptions; return; }

		//
		//  Debugging Control Functions
		//  ===========================
		//

		//  setDebugTrace
		//
		//  This function will turn on debugging for the span specified
		//
		//  PARAMETERS
		//
		//		size_t			-		Start offset for debugging
		//		size_t			-		End Offset for debugging
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void		setDebugTrace(size_t Start, size_t End) {
			DebugTrace = true;
			StartTraceWindow = Start;
			EndTraceWindow = End;
			return;
		}

		//  clearDebugTrace
		//
		//  This function will clear debugging
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void		clearDebugTrace() { DebugTrace = false; return; }


		//
		//  Auxilliary Functions
		//  ====================
		//

		//  reportStatistics
		//
		//  This function will report the accumulated statistics from the last function
		//
		//  PARAMETERS
		//
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void	reportStatistics() {
			double			Ratio = 0.0;
			os << "INFO: (De)Compress Input bytes: " << Stats.BytesIn << ", Output bytes: " << Stats.BytesOut << ", Tokens: " << Stats.Tokens << ", Reused Tokens: " << Stats.ReuseTokens << "." << std::endl;

			if (Stats.NS1Tokens > 0) Ratio = double(Stats.NS1Bits) / double(Stats.NS1Tokens);
			else Ratio = 0.0;
			os << "INFO: New Symbols (singlet): " << Stats.NS1Tokens << " were encoded in: " << Stats.NS1Bits << " bits (" << Ratio << " bits per symbol)." << std::endl;
			if (Stats.NS2Tokens > 0) Ratio = double(Stats.NS2Bits) / (double(Stats.NS2Tokens) * 2.0);
			else Ratio = 0.0;
			os << "INFO: New Symbols (doublet): " << Stats.NS2Tokens << " were encoded in: " << Stats.NS2Bits << " bits (" << Ratio << " bits per symbol)." << std::endl;
			if (Stats.NS3Tokens > 0) Ratio = double(Stats.NS3Bits) / (double(Stats.NS3Tokens) * 3.0);
			else Ratio = 0.0;
			os << "INFO: New Symbols (triplet): " << Stats.NS3Tokens << " were encoded in: " << Stats.NS3Bits << " bits (" << Ratio << " bits per symbol)." << std::endl;

			if (Stats.ES1Tokens > 0) Ratio = double(Stats.ES1Bits) / double(Stats.ES1Tokens);
			else Ratio = 0.0;
			os << "INFO: Existing Symbols (singlet): " << Stats.ES1Tokens << " were encoded in: " << Stats.ES1Bits << " bits (" << Ratio << " bits per symbol)." << std::endl;
			if (Stats.ES2Tokens > 0) Ratio = double(Stats.ES2Bits) / (double(Stats.ES2Tokens) * 2.0);
			else Ratio = 0.0;
			os << "INFO: Existing Symbols (doublet): " << Stats.ES2Tokens << " were encoded in: " << Stats.ES2Bits << " bits (" << Ratio << " bits per symbol)." << std::endl;
			if (Stats.ES3Tokens > 0) Ratio = double(Stats.ES3Bits) / (double(Stats.ES3Tokens) * 3.0);
			else Ratio = 0.0;
			os << "INFO: Existing Symbols (triplet): " << Stats.ES3Tokens << " were encoded in: " << Stats.ES3Bits << " bits (" << Ratio << " bits per symbol)." << std::endl;

			if (Stats.DictTokens > 0) Ratio = double(Stats.DictBits) / double(Stats.DictBytes);
			else Ratio = 0.0;
			os << "INFO: Dictionary References: " << Stats.DictTokens << " encoded " << Stats.DictBytes << " symbols in " << Stats.DictBits << " bits (" << Ratio << " bits per symbol)." << std::endl;
			if (Stats.StrTokens > 0) Ratio = double(Stats.StrBits) / double(Stats.StrBytes);
			else Ratio = 0.0;
			os << "INFO: Repeated Strings: " << Stats.StrTokens << " encoded " << Stats.StrBytes << " symbols in " << Stats.StrBits << " bits (" << Ratio << " bits per symbol)." << std::endl;
			if (Stats.RL8Tokens > 0) Ratio = double(Stats.RL8Bits) / double(Stats.RL8Bytes);
			else Ratio = 0.0;
			os << "INFO: RLE8 Runs: " << Stats.RL8Tokens << " encoded " << Stats.RL8Bytes << " symbols in " << Stats.RL8Bits << " bits (" << Ratio << " bits per symbol)." << std::endl;
			if (Stats.RL16Tokens > 0) Ratio = double(Stats.RL16Bits) / double(Stats.RL16Bytes);
			else Ratio = 0.0;
			os << "INFO: RLE16 Runs: " << Stats.RL16Tokens << " encoded " << Stats.RL16Bytes << " symbols in " << Stats.RL16Bits << " bits (" << Ratio << " bits per symbol)." << std::endl;
			if (Stats.RL32Tokens > 0) Ratio = double(Stats.RL32Bits) / double(Stats.RL32Bytes);
			else Ratio = 0.0;
			os << "INFO: RLE32 Runs: " << Stats.RL32Tokens << " encoded " << Stats.RL32Bytes << " symbols in " << Stats.RL32Bits << " bits (" << Ratio << " bits per symbol)." << std::endl;

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members                                                                                               *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Consfiguration
		std::ostream&	os;																			//  ostream for stats/debugging
		USHORT			WindowSize;																	//  Adaption window size
		SWITCHES		PermittedOptions;															//  Permitted compression options

		//  Debugging Controls
		bool			StatsTrace;																	//  Stats tracing state
		bool			DebugTrace;																	//  Debug tracing state
		size_t			StartTraceWindow;															//  Debugging trace window
		size_t			EndTraceWindow;																//  Ending trace window

		//  Statistics
		CStats			Stats;																		//  Statistics

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  canDoBetter
		//
		//  This function will determine if it is better to defeat the greedy algorithm at the current point
		//  in the compression.
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the input ByteStream
		//		size_t			-		The current best length that will be emitted
		//		DictRefCODEC&	-		Reference to the dictionary 
		//
		//  RETURNS
		//
		//		bool			-		If true then it is better to negate the current output selection.
		//
		//  NOTES
		//
		//

		bool	canDoBetter(ByteStream& bsIn, size_t CurrentBest, DictRefCODEC& Dictionary) {
			size_t			BestLength = 0;															//  New best length
			int				iDummy = 0;
			USHORT			usDummy = 0;

			//  Temporarily advance the position in the input stream
			bsIn.advance(1);

			//
			//  If enabled then attempt to use an existing dictionary string for the next chunk of input
			//

			if (PermittedOptions & DICPermitted) {
				BestLength = Dictionary.findLongestDictionaryString(bsIn, iDummy);
				if (BestLength > (CurrentBest + 1)) {
					bsIn.retreat(1);
					return true;
				}
			}

			//
			//  If enabled then attempt to use a string from the already encoded window in the buffer (LZ77)
			//

			if (PermittedOptions & LZPermitted) {
				BestLength = findLongestNewString(bsIn, usDummy);
				if (BestLength > (CurrentBest + 1)) {
					bsIn.retreat(1);
					return true;
				}
			}

			//
			//  If enabled then attempt to use a Run Length Encoding for the next chunk of input
			//

			if (PermittedOptions & RLEPermitted) {
				BestLength = findLongestRun(bsIn, iDummy);
				if (BestLength > (CurrentBest + 1)) {
					bsIn.retreat(1);
					return true;
				}
			}

			//  Retreat the stream to the original place
			bsIn.retreat(1);

			return false;
		}

		//  emitSymbol
		//
		//  This function will emit the passed extended symbol to the output stream
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the output ByteStream
		//		uint32_t		-		The extended symbol to be emitted
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void	emitSymbol(ByteStream& bsOut, uint32_t XSCode) {

			//  Emit the code symbol(s)
			if (XSCode >= (3 << 24)) bsOut.next(BYTE((XSCode & 0x00FF0000) >> 16));
			if (XSCode >= (2 << 24)) bsOut.next(BYTE((XSCode & 0x0000FF00) >> 8));
			bsOut.next(BYTE(XSCode & 0x000000FF));
			return;
		}

		//  findLongestNewString
		//
		//  This function will attempt to find the longest string within the already encoded byffer that matches the current input
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the input ByteStream
		//		USHORT&			-		Reference to the offset back in the buffer to the matching string
		//
		//  RETURNS
		//
		//		uint32_t		-		Length of the matching string
		//
		//  NOTES
		//
		//

		uint32_t		findLongestNewString(ByteStream& bsIn, USHORT& StrOffset) {
			size_t		WSz = 0;																			//  Search window size
			BYTE*		pSOW = bsIn.getPreReadWindow(64 * 1024, WSz);										//  Pointer to the start of the window
			BYTE*		pChunk = bsIn.getReadAddress();														//  Start of the chunk
			size_t		ChunkLen = bsIn.getRemainder();														//  Length of the chunk
			BYTE*		pBestString = nullptr;																//  Best string found so far
			uint32_t	BestLen = 0;																		//  Best string length so far
			uint32_t	cIndex = 0;																			//  Character compare index
			size_t		MinStringLen = 4;																	//  Minimum string length

			//  Clear the offset
			StrOffset = 0;

			//  If there is insufficient window then exit
			if (WindowSize < MinStringLen) return 0;

			//  Search the window for matching strings
			for (BYTE* pSearch = pChunk - MinStringLen; pSearch >= pSOW; pSearch--) {
				//  Compare the first character of the search with the first character of the chunk
				if (pSearch[0] == pChunk[0]) {
					for (cIndex = 1; cIndex < 256 && cIndex < (ChunkLen - 1); cIndex++) {
						if (pSearch[cIndex] != pChunk[cIndex]) break;
					}

					//  See if this is a candidate string
					if (cIndex >= MinStringLen) {
						if (cIndex > BestLen) {
							pBestString = pSearch;
							BestLen = cIndex;
						}
					}
				}
			}

			//  If no match was found indicate this to the caller
			if (BestLen == 0) return 0;

			//  Return the best (longest) string that was located
			StrOffset = USHORT(pChunk - pBestString);
			return BestLen;
		}

		//  findLongestRun
		//
		//  This function will attempt to find the longest run of 8, 16 or 32 bit sequences for the current input
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the input ByteStream
		//		int&			-		Reference to the run factor (8, 16 or 32 bit)
		//
		//  RETURNS
		//
		//		uint32_t		-		Length of the matching string
		//
		//  NOTES
		//
		//

		uint32_t		findLongestRun(ByteStream& bsIn, int& RunFactor) {
			uint32_t		Run8 = 1;																//  8 bit run
			uint32_t		Run16 = 2;																//  16 bit run
			uint32_t		Run32 = 4;																//  32 bit run
			BYTE*			pR8 = bsIn.getReadAddress();											//  Pointer for 8 bit run
			uint16_t*		pR16 = (uint16_t*)pR8;													//  Pointer for 16 bit run
			uint32_t*		pR32 = (uint32_t*)pR8;													//  Pointer for 32 bit run
			size_t			ChunkLen = bsIn.getRemainder();											//  Length of the chunk

			//  Clear the run factor
			RunFactor = 0;

			//  Compute the 8 bit run length
			while (pR8[0] == pR8[1] && Run8 < ChunkLen) {
				Run8++;
				pR8++;
			}

			//  Compute the 16 bit run length
			while (pR16[0] == pR16[1] && Run16 < ChunkLen) {
				Run16 += 2;
				pR16++;
			}

			//  Compute the 32 bit run length
			while (pR32[0] == pR32[1] && Run32 < ChunkLen) {
				Run32 += 4;
				pR32++;
			}

			//  Check for no runs possible
			if (Run8 + Run16 + Run32 == 7) return 0;

			//  Check for a possible 8 bit run (of minimum length)
			if (Run8 >= 4) {
				//  Preference is for an 8 bit run
				if (Run8 >= Run16 && Run8 >= Run32) {
					RunFactor = 8;
					return Run8;
				}
			}

			//  Check for a possible 16 bit run
			if (Run16 > 4) {
				if (Run16 >= Run32) {
					RunFactor = 16;
					return Run16;
				}
			}

			//  Check fora 32 bit run
			if (Run32 > 4) {
				RunFactor = 32;
				return Run32;
			}

			return 0;
		}

		//  findExtendedSymbol
		//
		//  This function will attempt to find the longest extended symbol that matches the current input
		//
		//  PARAMETERS
		//
		//		ByteStream&				-		Reference to the input ByteStream
		//		AdaptiveHuffManTree&	-		Reference to the Encoder
		//		uint32_t&				-		Reference to the extended symbol code
		//		DictRefCODEC&			-		Reference to the Dictionary store, encoder decoder
		//
		//  RETURNS
		//
		//		uint32_t				-		Length of the Extended Symbol
		//
		//  NOTES
		//
		//

		uint32_t		findExtendedSymbol(ByteStream& bsIn, AdaptiveHuffmanTree& Encoder, uint32_t& XSCode, DictRefCODEC& Dictionary) {
			size_t			XS2Count = 0;																		//  Count of matching doublets within window
			size_t			XS3Count = 0;																		//  Count of matching triplets within window
			BYTE*			pChunk = bsIn.getReadAddress();														//  Start of the chunk
			size_t			ChunkLen = bsIn.getRemainder();														//  Length of the chunk
			size_t			Threshold = 10;																		//  Selection threshold
			int				TempID = 0;																			//  Temporary Dictionary ID
			USHORT			TempOff = 0;																		//  Temporary offset
			uint32_t		TempSym = 0;																		//  Temporary symbol code

			//  Restrict the search to the max window (16k)
			if (ChunkLen > (16 * 1024)) ChunkLen = 16 * 1024;

			//  Set the default extended sequence code
			XSCode = (1 << 24) + bsIn.peek(0);

			//  Exit if the residual chunk is too small
			if (ChunkLen < 9) return 1;

			//  Count the number of matching triplets within the window
			TempSym = (3 << 24) + (bsIn.peek(0) << 16) + (bsIn.peek(1) << 8) + bsIn.peek(2);
			if (Encoder.hasEncoding(TempSym)) XS3Count = Threshold;
			else {
				for (BYTE* pSearch = pChunk + 3; pSearch < (pChunk + (ChunkLen - 6)); pSearch++) {
					if (memcmp(pSearch, pChunk, 3) == 0) {
						XS3Count++;
						pSearch += 2;
					}
				}
			}

			//  Count the number of matching doublets within the window
			TempSym = (2 << 24) + (bsIn.peek(0) << 8) + bsIn.peek(1);
			if (Encoder.hasEncoding(TempSym)) XS2Count = Threshold;
			else {
				for (BYTE* pSearch = pChunk + 2; pSearch < (pChunk + (ChunkLen - 4)); pSearch++) {
					if (memcmp(pSearch, pChunk, 2) == 0) {
						XS2Count++;
						pSearch++;
					}
				}
			}

			//
			//  String defeat prevention
			//
			//  If a string could start with the second or third symbol in the triplet
			//  or the second symbol in a doublet then prevent this condition
			//  

			if (XS2Count >= Threshold || XS3Count >= Threshold) {
				bsIn.advance(1);
				if (Dictionary.findLongestDictionaryString(bsIn, TempID) > 0) {
					XS2Count = 0;
					XS3Count = 0;
				}
				else {
					if (findLongestNewString(bsIn, TempOff) > 0) {
						XS2Count = 0;
						XS3Count = 0;
					}
				}
				bsIn.retreat(1);
			}

			if (XS3Count >= Threshold) {
				bsIn.advance(2);
				if (Dictionary.findLongestDictionaryString(bsIn, TempID) > 0) {
					XS3Count = 0;
				}
				else {
					if (findLongestNewString(bsIn, TempOff) > 0) {
						XS3Count = 0;
					}
				}
				bsIn.retreat(2);
			}

			//  Select triplet coding if it matches more characters and is above the threshold
			if ((XS3Count * 3) > (XS2Count * 2) && XS3Count >= Threshold) {
				XSCode = (3 << 24) + (bsIn.peek(0) << 16) + (bsIn.peek(1) << 8) + bsIn.peek(2);
				return 3;
			}

			//  Select triplet coding if it is above the threshold
			if (XS2Count >= Threshold) {
				XSCode = (2 << 24) + (bsIn.peek(0) << 8) + bsIn.peek(1);
				return 2;
			}

			//  Default
			return 1;
		}

		//  traceChunk
		//
		//  This function will log the chunk of the input buffer that is to be encoded
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the input ByteStream
		//		size_t			-		Length of buffer to be traced
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void		traceChunk(ByteStream& bsIn, size_t ChunkLen) {
			char		c = 0;																		//  Character to dispay

			//  Show tracing source string
			os << "TRACE: Source: '";

			//  Show the string
			for (size_t cIndex = 0; cIndex < ChunkLen; cIndex++) {
				c = bsIn.peek(cIndex);
				if (c < 32) c = '.';
				os << c;
			}

			os << "'";

			//  If this is a symbol then also show the raw encoding
			if (ChunkLen < 4) {
				os << " [";
				for (size_t cIndex = 0; cIndex < ChunkLen; cIndex++) {
					if (cIndex > 0) os << ",";
					os << int(bsIn.peek(cIndex));
				}
				os << "]";
			}

			os << "." << std::endl;

			//  Return to caller
			return;
		}

		//  traceChunk
		//
		//  This function will log the chunk of the input buffer that is to be encoded
		//
		//  PARAMETERS
		//
		//		BYTE*			-		Pointer to the chnk to be traced
		//		size_t			-		Length of buffer to be traced
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void		traceChunk(BYTE* pChunk, size_t ChunkLen) {
			char		c = 0;																		//  Character to dispay

			//  Show tracing source string
			os << "TRACE: Source: '";

			//  Show the string
			for (size_t cIndex = 0; cIndex < ChunkLen; cIndex++) {
				c = pChunk[cIndex];
				if (c < 32) c = '.';
				os << c;
			}

			os << "'";

			//  If this is a symbol then also show the raw encoding
			if (ChunkLen < 4) {
				os << " [";
				for (size_t cIndex = 0; cIndex < ChunkLen; cIndex++) {
					if (cIndex > 0) os << ",";
					os << int(pChunk[cIndex]);
				}
				os << "]";
			}

			os << "." << std::endl;

			//  Return to caller
			return;
		}


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Nested Classes                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   AdaptiveHuffmanTree Class																						*
		//*                                                                                                                 *
		//*   This class provides the implementation of the tree structure used for encoding.decoding variable length.		*
		//*   bit strings.                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class AdaptiveHuffmanTree {
		private:
			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Forward Declarations                                                                                          *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class HuffmanNode;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Nested Structures		                                                                                        *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Encoding LookUp Table entry

			typedef struct ELUTEntry {
				uint32_t			XSCode;														//  Extended Symbol Code
				HuffmanNode* pNode;														//  Pointer to the node in the tree
			} ELUTEntry;

		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  AdaptiveHuffmanTree  -  Normal Constructor
			//
			//  Creates a new Adaptive Huffman Tree in the given configuration
			//
			//  PARAMETERS
			//
			//		USHORT			-		Size of the aplphabet to be used
			//		USHORT			-		Adaption Window Size
			//		std::ostream&	-		Reference to the ostream to use for stats/debugging info
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			AdaptiveHuffmanTree(USHORT Alphabet, USHORT Window, std::ostream& TDStream) : os(TDStream) {
				size_t			iIndex;
				uint32_t		NXCode = (1 << 24) + Alphabet;												//  Not in use code

				DebugTrace = false;
				AlphabetSize = Alphabet;
				WindowSize = Window;

				//  Ensure that we have a minimum window
				if (WindowSize < 1024) WindowSize = 1024;

				//  Create an empty root node in the tree
				Root = new HuffmanNode(false, nullptr, nullptr, nullptr, 0, 0);

				//  Create and clear the Encoding Lookup Table
				ELUTCapacity = 1024;
				ELUTUsed = 0;
				ELUT1S = 0;
				ELUT2S = 0;
				ELUT3S = 0;

				//  Set the initial recording window position
				WindowPos = 0;

				//  Clear Last Encodon & Length
				LastEncodon = 0;
				LastCodeLen = 0;

				//  Clear the node count
				Nodes = 0;
				RecordingBuffer = nullptr;

				ELUT = (ELUTEntry*)malloc(ELUTCapacity * sizeof(ELUTEntry));

				if (ELUT == nullptr) {
					os << "ERROR: The Nuffman Tree failed to allocate the Encoding Lookup Table." << std::endl;
					return;
				}

				//  Clear the Encoding Lookup Table
				memset(ELUT, 0, ELUTCapacity * sizeof(HuffmanNode*));

				//  Create and clear the recording buffer
				RecordingBuffer = (uint32_t*)malloc(size_t(WindowSize) * sizeof(uint32_t));
				if (RecordingBuffer == nullptr) {
					os << "ERROR: The Nuffman Tree failed to allocate the Recording Buffer." << std::endl;
					return;
				}

				//  Clear the Recording Buffer
				for (iIndex = 0; iIndex < size_t(WindowSize); iIndex++) RecordingBuffer[iIndex] = NXCode;

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor	                                                                                                *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  AdaptiveHuffmanTree  -  Destructor
			//
			//  Destroys an existing Adaptive Huffman Tree
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			~AdaptiveHuffmanTree() {

				//  If there is at least a root then delete it - this will cascade the deletion to all nodes in the tree
				if (Root != nullptr) delete Root;

				//  Free the Encoding Lookup Table & Recording Buffer
				if (ELUT != nullptr) free(ELUT);
				if (RecordingBuffer != nullptr) free(RecordingBuffer);

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Members                                                                                                *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Functions                                                                                              *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  hasEncoding
			//
			//  Tests if the passed extended code symbol already exists in the tree, Reference counts are not incremented.
			//
			//  PARAMETERS
			//
			//		uint32_t		-		Extended Code Symbol
			//
			//  RETURNS
			//
			//		bool			-		true if the symbol already exists in the tree and the value was returned, otherwise false
			//
			//  NOTES
			//
			//

			bool	hasEncoding(uint32_t Symbol) {
				size_t			ELUTIndex = 0;														//  Index in the ELUT table

				//  Search the Encoding LookUp Table to find the node in the tree for the requested symbol
				ELUTIndex = findELUTEntry(Symbol);

				//  Not found conditions
				if (ELUTIndex == ELUTUsed) return false;
				if (ELUT[ELUTIndex].XSCode != Symbol) return false;

				//  Return to caller
				return true;
			}

			//  hasEncoding
			//
			//  Tests if the passed extended code symbol already exists in the tree, if so the encoded value is returned.
			//  The test if it succeeds will update the reference count of the symbol and potentially promote it's position in the tree.
			//
			//  PARAMETERS
			//
			//		uint32_t		-		Extended Code Symbol
			//		uint32_t&		-		Reference to the encoded value
			//		int&			-		Reference to the length in bits of the encoded value
			//
			//  RETURNS
			//
			//		bool			-		true if the symbol already exists in the tree and the value was returned, otherwise false
			//
			//  NOTES
			//
			//

			bool	hasEncoding(uint32_t Symbol, uint32_t& Encodon, int& EncLen) {
				uint32_t		NXCode = (1 << 24) + AlphabetSize;									//  Unused code
				size_t			ELUTIndex = 0;														//  Index in the ELUT table
				HuffmanNode*	pNode = nullptr;													//  Current Node

				//  Clear the encoded value
				Encodon = 0;
				EncLen = 0;

				//  Search the Encoding LookUp Table to find the node in the tree for the requested symbol
				ELUTIndex = findELUTEntry(Symbol);

				//  Not found conditions
				if (ELUTIndex == ELUTUsed) return false;
				if (ELUT[ELUTIndex].XSCode != Symbol) return false;

				//  Get the encoding for the symbol
				LastEncodon = Encodon = getEncoding(ELUT[ELUTIndex].pNode, EncLen);
				LastCodeLen = EncLen;

				//  Perform the bookeeping
				pNode = ELUT[ELUTIndex].pNode;
				if (RecordingBuffer[WindowPos] != NXCode) {
					size_t	DecIndex = findELUTEntry(RecordingBuffer[WindowPos]);
					(*(ELUT[DecIndex].pNode))--;
				}
				RecordingBuffer[WindowPos] = Symbol;
				WindowPos++;
				if (WindowPos == WindowSize) WindowPos = 0;
				(*pNode)++;
				promoteNode(pNode);

				//  Return to caller
				return true;
			}

			// insertSymbol
			//
			//  This function will add a new Symbol (Leaf Node) to the tree. The position that it is inserted at is determined by the
			//  current state of the tree and the Hits count for the symbol.
			//
			//  PARAMETERS
			//
			//		uint32_t	-		The Extended Symbol value to be added to the tree
			//		USHORT		-		the initial hit count to be set for the symbol
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void	insertSymbol(uint32_t NewSymbol, USHORT NewHits) {
				HuffmanNode*		InsertPoint = nullptr;														//  Suggested insertion point

				//  Get the insertion point
				InsertPoint = findInsertPoint(NewHits);

				//  Egregious failure --> Did not get an insertion point
				if (InsertPoint == nullptr) {
					os << "ERROR: Failed to find an insertion point for Symbol: " << NewSymbol << "." << std::endl;
					return;
				}

				//  If the target insertion point was a leaf then use the fork primitive to insert the new node 
				//  at that point in the tree structure, unless the leaf is a placeholder.
				if (InsertPoint->isLeaf()) {
					//  If the leaf is a placeholder then use it as is
					if (InsertPoint->getHits() == 0) {
						insertELUTEntry(NewSymbol, InsertPoint);
						InsertPoint->setSymbol(NewSymbol);
						InsertPoint->setHits(NewHits);
						return;
					}

					bool Direction = true;
					if (NewHits > InsertPoint->getHits()) Direction = false;
					insertELUTEntry(NewSymbol, InsertPoint->fork(Direction, NewSymbol, NewHits));
					return;
				}

				//  Insertion point is a branch, if it is unsaturated then use he unused path in the node
				//  otherwise fork the branch.
				if (InsertPoint->getZero() == nullptr) {
					InsertPoint->setZero(new HuffmanNode(true, InsertPoint, nullptr, nullptr, NewHits, NewSymbol));
					insertELUTEntry(NewSymbol, InsertPoint->getZero());
					return;
				}

				if (InsertPoint->getOne() == nullptr) {
					InsertPoint->setOne(new HuffmanNode(true, InsertPoint, nullptr, nullptr, NewHits, NewSymbol));
					insertELUTEntry(NewSymbol, InsertPoint->getOne());
					return;
				}

				//  Fork the current entry
				insertELUTEntry(NewSymbol, InsertPoint->fork(false, NewSymbol, NewHits));

				return;
			}

			//  getNextToken
			//
			//  This function will decode the next available token from the passed bit stream and return the decoded value
			//
			//  PARAMETERS
			//
			//		MSBitStrream&			-		Reference to the bit stream carrying the encoded data
			//
			//  RETURNS
			//
			//		uint32_t			-		The decoded extended symbol value
			//
			//  NOTES
			//
			//

			int32_t		getNextToken(MSBitStream& bsIn) {
				uint32_t			NextBit;
				HuffmanNode*		pNode = Root;													//  Current node in the tree
				uint32_t			NXCode = (1 << 24) + AlphabetSize;								// Unused code

				//  Loop until a leaf node is encounteresd or EOS detected
				LastCodeLen = 0;
				LastEncodon = 0;
				if (DebugTrace) os << "TRACE: getNextToken() read: '";
				while (!bsIn.eos()) {
					NextBit = bsIn.next(1);
					LastEncodon = (LastEncodon << 1) | NextBit;
					LastCodeLen++;
					//  Read a single bit from the stream
					if (DebugTrace) {
						if (NextBit == 0) os << "0";
						else os << "1";
					}
					if (NextBit == 0) pNode = pNode->getZero();										//  Move to the next node in the tree
					else pNode = pNode->getOne();
					if (pNode->isLeaf()) {
						//  Perform the bookeeping
						if (RecordingBuffer[WindowPos] != NXCode) {
							size_t	ELUTIndex = findELUTEntry(RecordingBuffer[WindowPos]);
							(*(ELUT[ELUTIndex].pNode))--;
						}
						RecordingBuffer[WindowPos] = pNode->getSymbol();
						WindowPos++;
						if (WindowPos == WindowSize) WindowPos = 0;
						(*pNode)++;
						promoteNode(pNode);
						if (DebugTrace) os << "', Symbol: " << pNode->getSymbol() << ", Hits: " << pNode->getHits() << "." << std::endl;
						return pNode->getSymbol();
					}
				}
				if (DebugTrace) os << " - OOOOPs forced EOS." << std::endl;
				//  End-Of-Stream encountered before the current token is acquired
				return (1 << 24) + EOS;
			}

			//  AdaptiveHuffmanTree  -  documentTree
			//
			//  This diagnostic function will dump the formatted content of the complete Huffman Tree to the defined stream.
			//  The dump is formatted as an indented explosion.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void	documentTree() {
				USHORT			sIndex;

				//  Show titles
				os << std::endl;
				os << "HUFFMAN TREE (0x" << this << ")" << std::endl;
				os << "------------" << std::endl;
				os << std::endl;

				//  Perform an indented explosion on each node in the tree
				if (Root == nullptr) os << "ERROR: No tree has been loaded." << std::endl;
				else documentNode(nullptr, *Root, 0);
				os << std::endl;

				//  Dump the Encoding Lookup Table
				os << "Encoding Lookup Table :-" << std::endl << std::endl;
				for (sIndex = 0; sIndex < ELUTUsed; sIndex++) {
					if (ELUT[sIndex].pNode != nullptr) os << " Symbol: " << (ELUT[sIndex].XSCode & 0x00FFFFFF) << " ----> Node: 0x" << ELUT[sIndex].pNode << "." << std::endl;
				}
				os << std::endl;

				//  Display the code lengths
				documentCodeLengths();

				//  Return to caller
				return;
			}

			//  getLastEncode
			//
			//  This diagnostic function will return the last encode/decode unit that was processed.
			//
			//  PARAMETERS
			//
			//		int&			-		Reference to the length of the coding unit
			//
			//  RETURNS
			//
			//		uint32_t		-		Last coding unit encountered
			//
			//  NOTES
			//
			//

			uint32_t	getLastEncode(int& CLen) {
				CLen = LastCodeLen;
				return LastEncodon;
			}

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Members                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Consfiguration
			USHORT			WindowSize;																	//  Adaption window size
			USHORT			AlphabetSize;																//  Alphabet Size
			std::ostream&	os;																			//  ostream for tracing and debugging

			//  Tree
			HuffmanNode*	Root;																		//  Root node of the tree
			USHORT			Nodes;																		//  Count of Nodes in the tree

			//  Encoding Lookup Table
			ELUTEntry* ELUT;																		//  Encoding looku[p table
			size_t			ELUTCapacity;																//  Capacity of the ELUT
			size_t			ELUTUsed;																	//  Number of entries in the ELUT
			size_t			ELUT1S;																		//  Number of L=1 symbols
			size_t			ELUT2S;																		//  Number of L=2 symbols
			size_t			ELUT3S;																		//  Number of L=3 symbols

			//  Window controls
			USHORT			WindowPos;																	//  Current position in window
			uint32_t*		RecordingBuffer;															//  Recording buffer

			//  Debugging Controls
			bool			DebugTrace;																	//  Tracing state
			uint32_t		LastEncodon;																//  Last encoded/decoded unit
			int				LastCodeLen;																//  Last encoded/decoded unit length

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  insertELUTEntry
			//
			//  This function will insert a new Encoding LookUp Table entry intu the ELUT.
			//
			//  PARAMETERS
			//
			//		unit32_t			-		The symbol that is to be inserted
			//		HuffmanNode*		-		Pointer to the Node in the tree containing the symbol
			//		
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void	insertELUTEntry(uint32_t NewSymbol, HuffmanNode* pNode) {
				int		SymbolClass = NewSymbol >> 24;														//  Symbol Class
				size_t	ELUTIndex = 0;																		//  Index into the ELUT

				//  Safety
				if (ELUT == nullptr) return;
				if (pNode == nullptr) return;

				//  Check that we have capacity for the new symbol - if not expand the ELUT
				if (ELUTUsed == ELUTCapacity) {
					ELUTCapacity += 256;
					ELUTEntry* NewELUT = (ELUTEntry*)realloc(ELUT, ELUTCapacity * sizeof(ELUTEntry));
					if (NewELUT == nullptr) {
						free(ELUT);
						os << "ERROR: The Huffman Tree failed to allocate the Encoding Lookup Table." << std::endl;
						return;
					}

					ELUT = NewELUT;
					memset(&ELUT[ELUTUsed], 0, (ELUTCapacity - ELUTUsed) * sizeof(ELUTEntry));
				}

				//  Find the point in the ELUT to insert the new entry
				ELUTIndex = findELUTEntry(NewSymbol);

				//  SNO - Check for an entry that is already in the table
				if (ELUTIndex < ELUTUsed && ELUT[ELUTIndex].XSCode == NewSymbol) return;

				//  If the insertion is not at the end of the table then open an entry in the table
				if (ELUTIndex < ELUTUsed) memmove(&ELUT[ELUTIndex + 1], &ELUT[ELUTIndex], (ELUTUsed - ELUTIndex) * sizeof(ELUTEntry));

				//  Add the new entry
				ELUT[ELUTIndex].XSCode = NewSymbol;
				ELUT[ELUTIndex].pNode = pNode;

				//  Bookeeping
				ELUTUsed++;
				if (SymbolClass == 1) ELUT1S++;
				else if (SymbolClass == 2) ELUT2S++;
				else ELUT3S++;

				//  Return to caller
				return;
			}

			//  findELUTEntry
			//
			//  This function will locate an existing ELUT entry or the point at which the entry should be inserted in the ELUT
			//
			//  PARAMETERS
			//
			//		unit32_t			-		The symbol that is to be located		
			//
			//  RETURNS
			//
			//		size_t				-		The index position of the entry in the ELUT
			//
			//  NOTES
			//
			//

			size_t	findELUTEntry(uint32_t Symbol) {
				int			SymbolClass = Symbol >> 24;														//  Symbol Class
				size_t		ELUTIndex = 0;																	//  Search start point

				//  Determine the start point for the search
				if (SymbolClass > 1) ELUTIndex += ELUT1S;
				if (SymbolClass > 2) ELUTIndex += ELUT2S;

				//  Linear search the ELUT until a matching (or greater) symbol is encountered
				for (; ELUTIndex < ELUTUsed; ELUTIndex++) {
					if (ELUT[ELUTIndex].XSCode >= Symbol) break;
				}

				//  Return the index position
				return ELUTIndex;
			}

			//  findInsertPoint
			//
			//  This function will locate the position in the tree to use for a new symbol.
			//  It will return a pointer to the Node that indicates where the new symbol should be placed, this could be :-
			//
			//		A pointer to a branch node with an unused branch
			//		A pointer to a leaf node that is unused (zero hits)
			//		A pointer to a leaf node must be forked to create the insert point
			//
			//  PARAMETERS
			//
			//		USHORT		-		the initial hit count to be set for the symbol
			//
			//  RETURNS
			//
			//		HuffmanNode*		-		Pointer to the Node in the tree where the new leaf should be inserted
			//
			//  NOTES
			//
			//

			HuffmanNode* findInsertPoint(USHORT NewHits) {
				HuffmanNode* InsertPoint = nullptr;												//  Insertion point

				//  SNO:  There is no root node, insert a new one and return it
				if (Root == nullptr) {
					if (DebugTrace) os << "SNO-ERROR: No root node availabe in the tree, recreating root." << std::endl;
					Root = new HuffmanNode(false, nullptr, nullptr, nullptr, 0, 0);
					return Root;
				}

				//  Recursively search for the best insertion point
				searchForInsertPoint(NewHits, 0, Root, &InsertPoint);

				//  Return the selected insert point
				return InsertPoint;
			}

			//  searchForInsertPoint
			//
			//  This function will search each node in the tree to locate the best node to use for a new symbol.
			//
			//  PARAMETERS
			//
			//		USHORT				-		the initial hit count to be set for the symbol
			//		int					-		Level of the current node in the tree
			//		HuffmanNode*		-		Pointer to the current node to be considered
			//		HuffmanNode**		-		Pointer to a pointer to the node, where the best node found so far will be returned
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void	searchForInsertPoint(USHORT NewHits, int Level, HuffmanNode* pNode, HuffmanNode** InsertPoint) {

				//  If there is no insert point located yet then the passed node will qualify
				if (*InsertPoint == nullptr) *InsertPoint = pNode;

				//  If a default qualifying node has been located then do no more
				if ((*InsertPoint)->isLeaf() && (*InsertPoint)->getHits() == 0) return;
				if ((*InsertPoint)->isBranch() && ((*InsertPoint)->getZero() == nullptr || (*InsertPoint)->getOne() == nullptr)) return;

				//  If the current node is an unsaturated branch then it automatically qualifies as the best option
				if (pNode->isBranch()) {
					if (pNode->getZero() == nullptr || pNode->getOne() == nullptr) {
						*InsertPoint = pNode;
						return;
					}
				}

				//  If the current branch is a leaf placeholder then it automatically qualifies as the best option
				if (pNode->isLeaf()) {
					if (pNode->getHits() == 0) {
						*InsertPoint = pNode;
						return;
					}
				}

				//  If we have not yet reached the ideal zone for insert then just pick the least hits node
				if ((*InsertPoint)->getHits() > NewHits) {
					if (pNode->getHits() < (*InsertPoint)->getHits()) *InsertPoint = pNode;
				}
				else {
					//  Any node with a hit count less than or equal to the target hit count is a possile candidate
					if (pNode->getHits() <= NewHits) {
						//  If the hit count is greater than the hit count for the current best candidate then this node becomes the best choise
						if (pNode->getHits() > (*InsertPoint)->getHits()) *InsertPoint = pNode;
						else {
							//  If the hit counts are equal and this node is at a higher level then it qualifies as the best option
							if (pNode->getHits() == (*InsertPoint)->getHits()) {
								if (pNode->getLevel() < (*InsertPoint)->getLevel()) *InsertPoint = pNode;
							}
						}
					}
				}

				//  Recurse into the children if this is a branch node
				if (pNode->isBranch()) {
					searchForInsertPoint(NewHits, Level + 1, pNode->getZero(), InsertPoint);
					if ((*InsertPoint)->isLeaf() && (*InsertPoint)->getHits() == 0) return;
					if ((*InsertPoint)->isBranch() && ((*InsertPoint)->getZero() == nullptr || (*InsertPoint)->getOne() == nullptr)) return;
					searchForInsertPoint(NewHits, Level + 1, pNode->getOne(), InsertPoint);
				}

				//  Return back up to the next level
				return;
			}

			//  getEncoding
			//
			//  Derive the encoding for a node in the tree by cascading up the tree identifying the access path
			//
			//	PARAMETERS:
			//
			//		HuffmanNode*			-		Pointer to the node for which the encoding will be drived
			//		int&					-		Reference to the length of the encoding bit string		
			//
			//	RETURNS:
			//
			//		uint32_t				-		The encoding for the symbol at the identified node
			//
			//  NOTES:
			//

			uint32_t	getEncoding(HuffmanNode* pNode, int& Length) {
				uint32_t		String = 0;																		//  Encoding string

				Length = 0;																						//  Clear the returned length

				//  Cascade up the parent chaine deriving the encoding
				while (!pNode->isRoot()) {
					Length++;
					String = (String >> 1) & 0x7FFFFFFF;														//  Latest bit defaults to '0'
					if (pNode->getParent()->getOne() == pNode) String = String | 0x80000000;					//  Latest bit overidden to '1'
					pNode = pNode->getParent();
				}

				//  Shift the string so that the LSB is at the low-order bit position
				String = String >> (32 - Length);

				//  Return the encoded bit string
				return String;
			}

			//  promoteNode
			//
			//  This function will promote a node in the tree the entire tree is searched for the highest node with the most hits <= the node hit count.
			//  This is the maximally greedy algorithm.
			//
			//  PARAMETERS
			//
			//		HuffmanNode*	-		Pointer to the node to be promoted
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void promoteNode(HuffmanNode* pNode) {
				HuffmanNode*		pPromoteTo = locatePromotePoint(pNode, pNode, Root);

				//  Determine if a swap should be made
				if (pPromoteTo != pNode) pNode->swap(*pPromoteTo);

				//  Percolate the promotion up to the root
				while (pNode->getLevel() > 2) {
					pNode = pNode->getParent();
					pPromoteTo = locatePromotePoint(pNode, pNode, Root);
					if (pPromoteTo != pNode) pNode->swap(*pPromoteTo);
				}

				//  Return to caller
				return;
			}

			//  locatePromotePoint
			//
			//  This function will recusively search the tree (indented explosion) to find the best qualifying node to [promote an updated node to.
			//
			//  PARAMETERS
			//
			//		HuffmanNode*	-		Pointer to the node to be promoted
			//		HuffmanNode*	-		Pointer to the best node located so far
			//		HuffmanNode*	-		Pointer to the current node to be considered
			//
			//  RETURNS
			//
			//		HuffmanNode*	-		Pointer to the best node located so far
			//
			//  NOTES
			//
			//

			HuffmanNode* locatePromotePoint(HuffmanNode* pRef, HuffmanNode* pBest, HuffmanNode* pSearch) {
				HuffmanNode*		pMyBest = pBest;

				//  To qualify the current node must have the same or less hits than the target node
				if (pSearch->getHits() <= pRef->getHits()) {
					//  If the node is at a higher level than the current best node then it supplants the best
					if (pSearch->getLevel() < pMyBest->getLevel()) pMyBest = pSearch;
				}

				//  If the current node is a branch then recurse into the structure
				if (pSearch->isBranch()) {
					if (pSearch->getZero() != nullptr) pMyBest = locatePromotePoint(pRef, pMyBest, pSearch->getZero());
					if (pSearch->getOne() != nullptr) pMyBest = locatePromotePoint(pRef, pMyBest, pSearch->getOne());
				}

				//  Return the best located so far
				return pMyBest;
			}

			//  AdaptiveHuffmanTree  -  documentNode
			//
			//  Recursive function to document a single node in the tree and recurse to the next node
			//
			//	PARAMETERS:
			//
			//		ostream&			-		Reference to the ostream to produce the output on
			//		HuffmanNode*		-		Pointer to the parent of the current node
			//		HuffmanNode&		-		Reference to the node to be documented
			//		int					-		Current level of the exlosion
			//
			//	RETURNS:
			//
			//		bool				-		if true signals to the parent to terminate the explosion, false continue
			//
			//  NOTES:
			//

			bool	documentNode(HuffmanNode* pParent, HuffmanNode& Node, int Level) {
				int			iIndex;
				int			SymbolClass = 0;

				//  Recursion protection - if the levels greater than 256 this is a malformed (probably looping) tree
				//  Signal the parent to terminate the explosion.
				if (Level > 256) {
					os << "ERROR: The Huffman Tree is malformed, probably contains loops - terminating the explosion." << std::endl;
					return true;
				}

				//  Document the NODE
				//  First output the indentation markers
				os << " ";
				for (iIndex = 0; iIndex < Level; iIndex++) os << ".";
				os << " ";

				//  Show the node type and identity
				if (Node.isRoot()) os << "Root Node (0x" << &Node << "), ";
				else {
					if (Node.isLeaf()) os << "Leaf Node (0x" << &Node << "), ";
					else os << "Branch Node (0x" << &Node << "), ";
				}

				//  Show parent
				os << "Parent Node (0x" << Node.getParent() << ") - ";
				os << "Level: " << Node.getLevel();

				//  Show the Node contents
				if (Node.isLeaf()) {
					if (Node.getHits() > 0) {
						SymbolClass = Node.getSymbol() >> 24;
						os << ", Class: " << SymbolClass;
						os << ", Symbol: " << (Node.getSymbol() & 0x00FFFFFF);
						os << ", Hits: " << Node.getHits();
						os << ", Leaves: " << Node.getLeaves();

						//  Show the encoding string and length
						int				CodeLength;
						uint32_t		CodeString;

						//  Get the encoding for the current node in the tree
						CodeString = getEncoding(&Node, CodeLength);

						os << ", code: (" << CodeString << ") '";

						//  Shift back MSB to absolute MSB
						CodeString = CodeString << (32 - CodeLength);
						for (iIndex = 0; iIndex < CodeLength; iIndex++) {
							if (CodeString & 0x80000000) os << "1";
							else os << "0";
							CodeString = CodeString << 1;
						}
						os << "', Length: " << CodeLength << " bits";
					}
					else os << " Empty placeholder leaf node";
				}
				else os << ", Zero: 0x" << Node.getZero() << ", " << "One: 0x" << Node.getOne() << ", Hits: " << Node.getHits() << ", Leaves: " << Node.getLeaves();
				os << "." << std::endl;

				//  Integrity checks
				if (Node.getParent() != pParent) {
					os << "ERROR: Integrity failure this node should point to 0x" << pParent << " as the parent node." << std::endl;
				}

				if (Node.getLevel() != Level) {
					os << "ERROR: Integrity failure this node should be at level: " << Level << ", not at level: " << Node.getLevel() << " in the tree." << std::endl;
				}

				//
				//  Indented explosion
				//
				//  Recurse into the Zero node then into the One node.
				//  Percolate a stop signal.
				//
				if (Node.getZero() != nullptr) {
					if (documentNode(&Node, *Node.getZero(), Level + 1)) return true;
				}

				if (Node.getOne() != nullptr) {
					if (documentNode(&Node, *Node.getOne(), Level + 1)) return true;
				}

				//  Return to caller (next level up can continue with the explosion)
				return false;
			}

			//  AdaptiveHuffmanTree  -  documentCodeLengths
			//
			//  This function will display the code lengths against the nodes in descending frequency order
			//
			//	PARAMETERS:
			//
			//	RETURNS:
			//
			//  NOTES:
			//

			void	documentCodeLengths() {
				USHORT			Leaves = Root->getLeaves();															//  Number of leaves
				USHORT			nIndex;																				//  Node index
				USHORT			lIndex;																				//  Leaf index
				HuffmanNode** PQ;																					//  Priority queue
				USHORT			Swaps = 1;																			//  Count of swaps
				HuffmanNode* Temp;																				//  Swap intermediate

				//  Check for degenerate tree
				if (Leaves == 0) {
					os << "Tree has no leaves - no code length table." << std::endl;
					return;
				}

				//  Allocate the priority queue for the leaf nodes
				PQ = (HuffmanNode**)malloc(Leaves * sizeof(HuffmanNode*));
				if (PQ == nullptr) return;

				//  Make a pass over the ELUT to capture the leaves
				lIndex = 0;
				for (nIndex = 0; nIndex < ELUTUsed; nIndex++) {
					if (ELUT[nIndex].pNode != nullptr) PQ[lIndex++] = ELUT[nIndex].pNode;
				}

				//  Bubble sort the priority queue into descending order of hits
				while (Swaps > 0) {
					Swaps = 0;
					for (nIndex = 0; nIndex < (Leaves - 1); nIndex++) {
						for (lIndex = nIndex + 1; lIndex < Leaves; lIndex++) {
							if (PQ[lIndex]->getHits() > PQ[nIndex]->getHits()) {
								Temp = PQ[nIndex];
								PQ[nIndex] = PQ[lIndex];
								PQ[lIndex] = Temp;
								Swaps++;
							}
						}
					}
				}

				//  Print out the Code Length Table
				os << "Code Lengths" << std::endl;
				os << std::endl;
				for (lIndex = 0; lIndex < Leaves; lIndex++) {
					os << "Symbol: " << (PQ[lIndex]->getSymbol() & 0x00FFFFFF) << ", Hits: " << PQ[lIndex]->getHits() << ", Code Length: " << PQ[lIndex]->getLevel() << "." << std::endl;
				}
				os << std::endl;

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Nested Classes                                                                                                *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   HuffmanNode Class																								*
			//*                                                                                                                 *
			//*   This class represents a single node (leaf or branch) in the Huffman encoding tree structure.					*
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class HuffmanNode {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  HuffmanNode  -  Constructor
				//
				//  Constructs a new Huffman node and optionally plugs it dirsctly into the Huffman Tree structure.
				//
				//	PARAMETERS:
				//
				//		bool				-		If true then the node is a leaf node, otherwise it is a branch
				//		HuffmanNode*		-		Pointer to the parent node of the new node
				//		HuffmanNode*		-		Pointer to the zero path child node of the new node
				//		HuffmanNode*		-		Pointer to the one path child node of the new node
				//		USHORT				-		Initial value of the Hit counter for the node
				//		uint32_t			-		The symbol value that this (leaf) node is encoding
				//
				//	RETURNS:
				//
				//  NOTES:
				//

				HuffmanNode(bool MakeLeaf, HuffmanNode* Newparent, HuffmanNode* NewZero, HuffmanNode* NewOne, USHORT NewHits, uint32_t NewSymbol) {
					//  Set the node type
					Leaf = MakeLeaf;

					//  Set the parent pointer
					Parent = Newparent;

					//  Clear the child pointers
					Zero = nullptr;
					One = nullptr;

					//  Set the level in the structure
					if (Parent == nullptr) Level = 0;
					else Level = Parent->getLevel() + 1;

					//  Make settings that are conditional on the type of node
					if (Leaf) {
						//  The node is a leaf then set the symbol and the hit and leaves counts, the counts are percolated up to the root
						Symbol = NewSymbol;
						Hits = 0;
						setHits(NewHits);
						Leaves = 0;
						setLeaves(1);
					}
					else {
						//  The node is a branch clear the symbol and hit and leaves count, these will be accumulated from the children (if any)
						Symbol = 0;
						Hits = 0;
						Leaves = 0;
						setZero(NewZero);
						setOne(NewOne);

						//  Set the level to cause it to cascade down through the tree
						setLevel(Level);
					}

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Destructor	                                                                                                *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//	destructor
				//
				//	Destroys a HuffmanNode object and propogates the deletion to any chidren.
				//
				//	PARAMETERS:
				//
				//	RETURNS:
				//
				//  NOTES:
				//

				~HuffmanNode() {
					//  Delete any children
					if (Zero != nullptr) delete Zero;
					if (One != nullptr) delete One;
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Members                                                                                                *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Node Type Accessors
				bool				isLeaf() { return Leaf; }
				bool				isBranch() { return !Leaf; }
				bool				isRoot() { if (Parent == nullptr) return true; return false; }
				void				makeLeaf() { Leaf = true; return; }
				void				makeLeaf(uint32_t NewSymbol) { Symbol = NewSymbol; Leaf = true; return; }
				void				makeBranch() { Leaf = false; return; }

				//  Structure & Value Accessors
				HuffmanNode*		getParent() { return Parent; }
				HuffmanNode*		getOne() { return One; }
				HuffmanNode*		getZero() { return Zero; }
				USHORT				getLevel() { return Level; }
				USHORT				getHits() { return Hits; }
				USHORT				getLeaves() { return Leaves; }
				uint32_t			getSymbol() { return Symbol; }
				void				setSymbol(uint32_t NewSymbol) { Symbol = NewSymbol; return; }

				void				setParent(HuffmanNode* Newparent) { Parent = Newparent; return; }

				//  HuffmanNode  -  setZero
				//
				//  Sets the zero path pointer of the branch node to the passed value, this also sets a new count for the leaves and hits in the current node and above
				//  It will also set the level in the new children.
				//
				//	PARAMETERS:
				//
				//		HuffmanNode*		-		Pointer to the zero path child node of the new node
				//
				//	RETURNS:
				//
				//  NOTES:
				//

				void	setZero(HuffmanNode* NewZero) {
					USHORT				NewCount;																			//  New hit or leaf count

					//  Not appropriate for a leaf node
					if (Leaf) return;

					//  Set the new pointer
					Zero = NewZero;

					//  Recompute the hit counts for this node and percolate the change
					NewCount = 0;
					if (Zero != nullptr) NewCount += Zero->Hits;
					if (One != nullptr) NewCount += One->Hits;
					setHits(NewCount);

					//  Recompute the leaf counts for this node and percolate the change
					NewCount = 0;
					if (Zero != nullptr) NewCount += Zero->Leaves;
					if (One != nullptr) NewCount += One->Leaves;
					setLeaves(NewCount);

					//  Cascade the level setting down through the zero path
					if (Zero != nullptr) Zero->setLevel(Level + 1);

					//  Return to caller
					return;
				}

				//  HuffmanNode  -  setOne
				//
				//  Sets the one path pointer of the branch node to the passed value, this also sets a new count for the leaves and hits in the current node and above
				//  It will also set the level in the new children.
				//
				//	PARAMETERS:
				//
				//		HuffmanNode*		-		Pointer to the zone path child node of the new node
				//
				//	RETURNS:
				//
				//  NOTES:
				//

				void setOne(HuffmanNode* NewOne) {
					USHORT				NewCount;																			//  New hit or leaf count

					//  Not appropriate for a leaf node
					if (Leaf) return;

					//  Set the new pointer
					One = NewOne;

					//  Recompute the hit counts for this node and percolate the change
					NewCount = 0;
					if (Zero != nullptr) NewCount += Zero->Hits;
					if (One != nullptr) NewCount += One->Hits;
					setHits(NewCount);

					//  Recompute the leaf counts for this node and percolate the change
					NewCount = 0;
					if (Zero != nullptr) NewCount += Zero->Leaves;
					if (One != nullptr) NewCount += One->Leaves;
					setLeaves(NewCount);

					//  Cascade the level setting down through the one path
					if (One != nullptr) One->setLevel(Level + 1);

					//  Return to caller
					return;
				}

				//  HuffmanNode  -  setLevel
				//
				//  Sets the level of the current node, it will also cascade the setting down to the level below
				//
				//	PARAMETERS:
				//
				//		USHORT				-		New level to be sed
				//
				//	RETURNS:
				//
				//  NOTES:
				//

				void setLevel(USHORT NewLevel) {
					//  Set the new level
					Level = NewLevel;

					//  If this is a branch node then cascade the change to the next level down
					if (isBranch()) {
						if (Zero != nullptr) Zero->setLevel(Level + 1);
						if (One != nullptr) One->setLevel(Level + 1);
					}

					//  Return to caller
					return;
				}

				//  HuffmanNode  -  setHits
				//
				//  Sets the number of hits on the current node and percolates the change up to the root of the tree
				//
				//	PARAMETERS:
				//
				//		USHORT				-		New Hit Count
				//
				//	RETURNS:
				//
				//  NOTES:
				//

				void setHits(USHORT NewHits) {
					short				DeltaHits = NewHits - Hits;														//  Delta to be percolated to the root
					HuffmanNode*		pParent = Parent;																//  Pointer to parent

					//  Set the hit count in the current node
					Hits = NewHits;

					//  Percolate the change up the structure to the root
					while (pParent != nullptr) {
						pParent->Hits += DeltaHits;
						pParent = pParent->Parent;
					}

					//  Return to caller
					return;
				}

				//  HuffmanNode  -  setLeaves
				//
				//  Sets the count of leaves at and below on the current node and percolates the change up to the root of the tree
				//
				//	PARAMETERS:
				//
				//		USHORT				-		New count of leaves
				//
				//	RETURNS:
				//
				//  NOTES:
				//

				void setLeaves(USHORT NewLeaves) {
					short				DeltaLeaves = NewLeaves - Leaves;												//  Delta to be percolated to the root
					HuffmanNode*		pParent = Parent;																//  Pointer to parent

					//  Set the leaf count in the current node
					Leaves = NewLeaves;

					//  Percolate the change up to the root node
					while (pParent != nullptr) {
						pParent->Leaves += DeltaLeaves;
						pParent = pParent->Parent;
					}

					//  Return to caller
					return;
				}

				//  HuffmanNode  -  operator ++  Prefix increment
				//
				//  Increments the current hit counter and percolates the change up the tree
				//
				//	PARAMETERS:
				//
				//	RETURNS:
				//
				//		HuffmanNode&		-		Reference to the current node
				//
				//  NOTES:
				//

				HuffmanNode& operator ++() {
					setHits(Hits + 1);
					return *this;
				}

				//  HuffmanNode  -  operator ++  Postfix increment
				//
				//  Increments the current hit counter and percolates the change up the tree
				//
				//	PARAMETERS:
				//
				//	RETURNS:
				//
				//		HuffmanNode		-		The pre-modified version of the current node
				//
				//  NOTES:
				//

				HuffmanNode	operator++(int) {
					HuffmanNode			PreMod(*this);

					setHits(Hits + 1);
					return PreMod;
				}

				//  HuffmanNode  -  operator --  Prefix decrement
				//
				//  Decrements the current hit counter and percolates the change up the tree
				//
				//	PARAMETERS:
				//
				//	RETURNS:
				//
				//		HuffmanNode&		-		Reference to the current node
				//
				//  NOTES:
				//

				HuffmanNode& operator --() {
					if (Hits == 0) return *this;
					setHits(Hits - 1);
					return *this;
				}

				//  HuffmanNode  -  operator --  Postfix decrement
				//
				//  Decrements the current hit counter and percolates the change up the tree
				//
				//	PARAMETERS:
				//
				//	RETURNS:
				//
				//		HuffmanNode		-		The pre-modified version of the current node
				//
				//  NOTES:
				//

				HuffmanNode	operator --(int) {
					HuffmanNode			PreMod(*this);

					if (Hits == 0) return *this;
					setHits(Hits - 1);
					return PreMod;
				}

				//  Structure manipulators

				//  HuffmanNode  -  swap
				//
				//  Exhanges the position in the tree structure of the current (source) node with the passed (target) node.
				//
				//	PARAMETERS:
				//
				//		HuffmanNode&		-		Reference to target node for the exchange
				//
				//	RETURNS:
				//
				//  NOTES:
				//

				void	swap(HuffmanNode& nTarget) {
					HuffmanNode* sParent = Parent;													//  Pointer to the parent of the source node
					HuffmanNode* tParent = nTarget.getParent();										//  Pointer to the parent of the target node

					//  If the two nodes are on the same parent then update 
					if (tParent == sParent) {
						if (tParent->getZero() == &nTarget) {
							tParent->setZero(this);
							sParent->setOne(&nTarget);
						}
						else {
							tParent->setOne(this);
							sParent->setZero(&nTarget);
						}
						return;
					}

					//  Update the target node
					if (tParent->getZero() == &nTarget) tParent->setZero(this);
					else tParent->setOne(this);
					nTarget.setParent(sParent);

					//  Update the source node
					if (sParent->getZero() == this) sParent->setZero(&nTarget);
					else sParent->setOne(&nTarget);
					setParent(tParent);

					//  Return to caler
					return;
				}

				//  HuffmanNode  -  fork
				//
				//  Inserts a new branch node at the position of this node in the tree making this node a child of the new branch.
				//  A new leaf node is generated on the 'spare' path on the new branch node.
				//
				//	PARAMETERS:
				//
				//		bool		-		The path for the new leaf node, false ==> zero and true ==> one
				//		uint32_t	-		The new symbol to be encoded in the new leaf
				//		USHORT		-		The initial value for the hit counter in the new leaf
				//
				//	RETURNS:
				//
				//  NOTES:
				//

				HuffmanNode* fork(bool Direction, uint32_t NewSymbol, USHORT NewHits) {
					HuffmanNode*	pNewLeaf;																//  Pointer to the new leaf node
					HuffmanNode*	pNewBranch;																//  Pointer to the new branch node
					HuffmanNode*	pParent = Parent;														//  Pointer to the parent node of this one

					//  Create the new leaf node
					pNewLeaf = new HuffmanNode(true, nullptr, nullptr, nullptr, NewHits, NewSymbol);

					//  Create the new branch node
					if (Direction) pNewBranch = new HuffmanNode(false, pParent, this, pNewLeaf, 0, 0);
					else pNewBranch = new HuffmanNode(false, pParent, pNewLeaf, this, 0, 0);

					//  Set the pointer in the parent to the new branch
					if (pParent->getZero() == this) pParent->setZero(pNewBranch);
					else pParent->setOne(pNewBranch);

					//  Set the parent pointers in the two child nodes
					setParent(pNewBranch);
					pNewLeaf->setParent(pNewBranch);

					//  Return the pointer to the new leaf node
					return pNewLeaf;
				}

			private:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Private Members                                                                                               *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Node type
				bool					Leaf;																//  true if this is a leaf node

				//  Tree Structures
				HuffmanNode*			Parent;																//  Parent node
				HuffmanNode*			Zero;																//  Node for a zero bit
				HuffmanNode*			One;																//  Node for a one bit
				USHORT					Level;																//  Level of this node in the tree

				//  Symbol for a leaf node
				uint32_t				Symbol;																//  Extended Symbol encoded by this node

				//  Statistics
				USHORT					Hits;																//  Hit count (within window)
				USHORT					Leaves;																//  Number of leaves at and below this level

			};
		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   OffsetCODEC Class																								*
		//*                                                                                                                 *
		//*   This class provides the implementation of the Offset Encoder and Decoder.										*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class OffsetCODEC {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  OffsetCODEC  -  Default Constructor
			//
			//  Constructs a new OffsetCODEC object 
			//
			//	PARAMETERS:
			//
			//		std::ostream&			-			Reference to the stream for reporting
			//
			//	RETURNS:
			//
			//  NOTES:
			//

			OffsetCODEC(std::ostream& TDStream) : os(TDStream) {

				memset(ArenaHits, 0, 64 * sizeof(size_t));
				memset(APQ, 0, 64 * sizeof(int));

				//  Initialise the Arena ranking
				for (int aIndex = 0; aIndex < 64; aIndex++) {
					ArenaHits[aIndex] = 0;
					APQ[aIndex] = BYTE(aIndex);
				}

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor	                                                                                                *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//	destructor
			//
			//	Destroys an OffsetCODEC object.
			//
			//	PARAMETERS:
			//
			//	RETURNS:
			//
			//  NOTES:
			//

			~OffsetCODEC() {
				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Members                                                                                                *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Functions                                                                                              *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  hasEncoding
			//
			//  Returns the encoding for the passed offset, it also maintains the hit count and ranking for the arena.
			//
			//  PARAMETERS
			//
			//		USHORT			-		Offset
			//		uint32_t&		-		Reference to the encoded value
			//		int&			-		Reference to the length in bits of the encoded value
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void	hasEncoding(USHORT Offset, uint32_t& Encodon, int& EncLen) {
				int			ArenaNo = Offset / 1024;													//  Arena Number
				int			Residue = Offset % 1024;													//  Residue (Offset within arena
				int			ArenaRank = 0;																//  Arena ranking

				//  Determine the current arena ranking
				for (ArenaRank = 0; ArenaRank < 64; ArenaRank++) {
					if (APQ[ArenaRank] == ArenaNo) break;
				}

				//  VL1 Encoding ... 
				switch (ArenaRank) {
				case 0:
					Encodon = 0;
					EncLen = 2;
					break;

				case 1:
					Encodon = 2;
					EncLen = 3;
					break;

				case 2:
					Encodon = 6;
					EncLen = 4;
					break;

				case 3:
					Encodon = 28;
					EncLen = 6;
					break;

				case 4:
					Encodon = 29;
					EncLen = 6;
					break;

				case 5:
					Encodon = 30;
					EncLen = 6;
					break;

				case 6:
					Encodon = 31;
					EncLen = 6;
					break;

				default:
					Encodon = (1 << 6) + ArenaRank;
					EncLen = 7;
					break;
				}

				Encodon = (Encodon << 10);

				//  Add the residual offset to the encoding
				Encodon += Residue;
				EncLen += 10;

				//  Update the hit count
				ArenaHits[ArenaNo]++;

				//  Promote the entry up the priority queue
				for (; ArenaRank >= 1; ArenaRank--) {
					if (ArenaHits[APQ[ArenaRank]] > ArenaHits[APQ[ArenaRank - 1]]) {
						//  Swap rankings
						Residue = APQ[ArenaRank - 1];
						APQ[ArenaRank - 1] = APQ[ArenaRank];
						APQ[ArenaRank] = Residue;
					}
					else break;
				}

				//  Return to caller
				return;
			}

			//  getNextToken
			//
			//  This function will decode the next available offset from the passed bit stream and return the decoded value
			//
			//  PARAMETERS
			//
			//		MSBitStrream&			-		Reference to the bit stream carrying the encoded data
			//
			//  RETURNS
			//
			//		USHORT			-		The decoded offset
			//
			//  NOTES
			//
			//

			USHORT	getNextToken(MSBitStream& IBS) {
				int			ArenaRank = 0;															//  Arena Ranking
				int			Temp = 0;																//  Temporary swap area
				USHORT		Offset = 0;																//  Final Offset

				//  Read the Arena Ranking - using the VL1 scheme
				if (IBS.next(1) == 1) ArenaRank = IBS.next(6);
				else {
					if (IBS.next(1) == 0) ArenaRank = 0;
					else {
						if (IBS.next(1) == 0) ArenaRank = 1;
						else {
							if (IBS.next(1) == 0) ArenaRank = 2;
							else ArenaRank = IBS.next(2) + 3;
						}
					}
				}

				//  Compute the offset
				Offset = USHORT((APQ[ArenaRank] * 1024) + IBS.next(10));

				//  Update the hit count
				ArenaHits[APQ[ArenaRank]]++;

				//  Promote the entry up the priority queue
				for (; ArenaRank >= 1; ArenaRank--) {
					if (ArenaHits[APQ[ArenaRank]] > ArenaHits[APQ[ArenaRank - 1]]) {
						//  Swap rankings
						Temp = APQ[ArenaRank - 1];
						APQ[ArenaRank - 1] = APQ[ArenaRank];
						APQ[ArenaRank] = Temp;
					}
					else break;
				}

				//  Return the offset to the caller
				return Offset;
			}

			//  showStatistics
			//
			//  This function will report the statistics of hits in the arenas
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void	showStatistics() {
				//  Report on the OffsetCODEC Arena Hits
				os << "INFO: Offset Encoding Statistics follow :-" << std::endl;
				for (int aIndex = 0; aIndex < 64; aIndex++) {
					os << "  " << (aIndex + 1) << ". Arena: " << APQ[aIndex] << ", Base Offset: " << (APQ[aIndex] * 1024) << ", HITS: " << ArenaHits[APQ[aIndex]] << "." << std::endl;
				}
			}

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Members                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Arena Ranking
			size_t			ArenaHits[64];																	//  Arena Hit Counters
			int				APQ[64];																		//  Arena Priority Queue

			std::ostream& os;																				//  Stream for reporting

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   DictRefCODEC Class																							*
		//*                                                                                                                 *
		//*   This class provides the implementation of the Dictionary Reference Encoder and Decoder.						*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class DictRefCODEC {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  DictRefCODEC  -  Default Constructor
			//
			//  Constructs a new DictRefCODEC object, the underlying dictionary and it's priority queue are lazy initialised.
			//
			//	PARAMETERS:
			//
			//		std::ostream&			-			Reference to the stream for reporting
			//
			//	RETURNS:
			//
			//  NOTES:
			//

			DictRefCODEC(std::ostream& TDStream) : pDictionary(nullptr), DPQ(nullptr), os(TDStream) {

				//  Clear the members to the base state
				DictBits = 0;
				DictCapacity = 0;
				DictEntries = 0;
				memset(NDicBits, 0, 16 * sizeof(size_t));
				memset(EDicBits, 0, 16 * sizeof(size_t));

				//  Clear the statistics
				EncDecs = 0;
				TotalBits = 0;
				FixedEncs = 0;
				for (int sIndex = 0; sIndex < 16; sIndex++) {
					NDicBits[sIndex] = 0;
					EDicBits[sIndex] = 0;
				}

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor	                                                                                                *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//	destructor
			//
			//	Destroys an DictRefCODEC object.
			//
			//	PARAMETERS:
			//
			//	RETURNS:
			//
			//  NOTES:
			//

			~DictRefCODEC() {

				//  Free the underlying dictionary and priority queue
				if (pDictionary != nullptr) free(pDictionary);
				if (DPQ != nullptr) free(DPQ);

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Members                                                                                                *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Functions                                                                                              *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  hasEncoding
			//
			//  Returns the encoding for the passed Dictionary Reference, it also maintains the hit count and ranking for the arena.
			//
			//  PARAMETERS
			//
			//		int				-		Dictionary Reference
			//		uint32_t&		-		Reference to the encoded value
			//		int&			-		Reference to the length in bits of the encoded value
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void	hasEncoding(int DictRef, uint32_t& Encodon, int& EncLen) {
				int			Rank = 0;																		//  Priority Ranking of the reference
				int			Temp = 0;																		// Temporary for swapping entries

				//  Update the Encode counter
				EncDecs++;

				//  Find the entry in the priority queue
				for (Rank = 0; Rank < DictEntries; Rank++) {
					if (DPQ[Rank] == DictRef) break;
				}

				//
				//  Send the ranking encoded as follows
				//
				//  If DictBits < 11 send DictBits of Rank
				//  else
				//    if Rank < 512 send 0 x xxxxxxxx  -  0 followed by 9 bits of rank
				//    else send 1 xxxx  -  1 followed by DictBits of rank
				//

				if (DictBits < 11) {
					Encodon = Rank;
					EncLen = DictBits;
					NDicBits[DictBits - 1]++;
				}
				else {
					if (Rank < 512) {
						Encodon = Rank;
						EncLen = 10;
						FixedEncs++;
					}
					else {
						Encodon = (1 << DictBits) + Rank;
						EncLen = DictBits + 1;
						EDicBits[DictBits - 1]++;
					}
				}

				TotalBits += EncLen;

				//  Increment the hit count for the reference
				pDictionary[DPQ[Rank]].Hits++;

				//  Promote the entry in the priority queue
				for (; Rank > 0; Rank--) {
					if (pDictionary[DPQ[Rank]].Hits > pDictionary[DPQ[Rank - 1]].Hits) {
						//  Swap the entries
						Temp = DPQ[Rank - 1];
						DPQ[Rank - 1] = DPQ[Rank];
						DPQ[Rank] = Temp;
					}
				}

				//  Return to caller
				return;
			}

			//  getNextToken
			//
			//  This function will decode the next available Dictionary Reference from the passed bit stream and return the decoded value
			//
			//  PARAMETERS
			//
			//		MSBitStrream&			-		Reference to the bit stream carrying the encoded data
			//
			//  RETURNS
			//
			//		int			-		The decoded Dictionary Reference
			//
			//  NOTES
			//
			//

			int		getNextToken(MSBitStream& IBS) {
				int			DictRef = 0;																	//  Decoded reference
				int			Rank = 0;																		//  Priority Ranking of the reference
				int			Temp = 0;																		// Temporary for swapping entries

				//  Update the decode counter
				EncDecs++;

				//
				//  The ranking was sent encoded as follows
				//
				//  If DictBits < 11 send DictBits of Rank
				//  else
				//    if Rank < 512 send 0 x xxxxxxxx  -  0 followed by 9 bits of rank
				//    else send 1 xxxx  -  1 followed by DictBits of rank
				//

				if (DictBits < 11) {
					Rank = IBS.next(DictBits);
					NDicBits[DictBits - 1]++;
					TotalBits += DictBits;
				}
				else {
					if (IBS.next(1) == 0) {
						Rank = IBS.next(9);
						FixedEncs++;
						TotalBits += 10;
					}
					else {
						Rank = IBS.next(DictBits);
						EDicBits[DictBits - 1]++;
						TotalBits += (DictBits + 1);
					}
				}

				//  Dummy encoding the reference is passed as-is
				DictRef = DPQ[Rank];

				//  Increment the hit count for the reference
				pDictionary[DPQ[Rank]].Hits++;

				//  Promote the entry in the priority queue
				for (; Rank > 0; Rank--) {
					if (pDictionary[DPQ[Rank]].Hits > pDictionary[DPQ[Rank - 1]].Hits) {
						//  Swap the entries
						Temp = DPQ[Rank - 1];
						DPQ[Rank - 1] = DPQ[Rank];
						DPQ[Rank] = Temp;
					}
				}

				//  Return the reference
				return DictRef;
			}

			//  getDictionaryString
			//
			//  This function will return the effective offset and length of a string from the string for the goven reference
			//
			//  PARAMETERS
			//
			//		int			-		The Dictionary Reference
			//		size_t&		-		Reference to the string length
			//
			//  RETURNS
			//
			//		size_t			-		The effective offset of the string in the buffer
			//
			//  NOTES
			//
			//

			size_t	getDictionaryString(int DictRef, uint32_t& StrLen) {
				StrLen = pDictionary[DictRef].Length;
				return pDictionary[DictRef].Offset;
			}

			//  findLongestDictionaryString
			//
			//  This function will attempt to find the longest possible dictionary string that matches the current input
			//
			//  PARAMETERS
			//
			//		ByteStream&		-		Reference to the input ByteStream
			//		int&			-		Reference to the dictionary ID for the selected string
			//
			//  RETURNS
			//
			//		uint32_t		-		Length of the selected dictionary item
			//
			//  NOTES
			//
			//

			uint32_t		findLongestDictionaryString(ByteStream& bsIn, int& DICID) {
				BYTE*		pBfr = bsIn.getBufferAddress();													//  Start of buffer address
				BYTE*		pChunk = bsIn.getReadAddress();													//  Pointer to the current chunk in the buffer
				size_t		ChunkLen = bsIn.getRemainder();													//  Length of the chunk

				//  Clear the dictionary ID
				if (ChunkLen == 0) {
					DICID = 0;
					return 0;
				}

				DICID = -1;

				//  Iterate over the dictionary finding the longest matching entry
				for (int dIndex = 0; dIndex < DictEntries; dIndex++) {
					//  See if the first character of the dictionary string matches the first character of the chunk
					if (pBfr[pDictionary[dIndex].Offset] == *pChunk) {
						//  See if the chunk is long enough
						if (ChunkLen >= pDictionary[dIndex].Length) {
							if (memcmp(&pBfr[pDictionary[dIndex].Offset], pChunk, pDictionary[dIndex].Length) == 0) {
								if (DICID == -1) DICID = dIndex;
								else {
									if (pDictionary[dIndex].Length > pDictionary[DICID].Length) DICID = dIndex;
								}
							}
						}
					}
				}

				//  Return the length of the longest match
				if (DICID == -1) return 0;
				return pDictionary[DICID].Length;
			}

			//  addToDictionary
			//
			//  Adds the passed entry to the dictionary
			//
			//  PARAMETERS
			//
			//		size_t			-		Offset (in the buffer) to the string
			//		size_t			-		Size of the string
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void	addToDictionary(size_t Offset, size_t Length) {

				//  If the dictionary has not yet been allocated then allocate an initial page
				if (pDictionary == nullptr) {
					pDictionary = (DictionaryEntry*)malloc(DictPageSize * sizeof(DictionaryEntry));
					if (pDictionary == nullptr) return;
					DictCapacity = DictPageSize;

					DPQ = (int*)malloc(DictPageSize * sizeof(int));
					if (DPQ == nullptr) {
						free(pDictionary);
						pDictionary = nullptr;
						DictCapacity = 0;
						return;
					}
				}
				else {
					//  Check that there is enough capacity in the dictionary for a new entry - if not expand the dictionary
					if (DictEntries == DictCapacity) {
						DictCapacity += DictPageSize;
						DictionaryEntry* pNewDictionary = (DictionaryEntry*)realloc(pDictionary, DictCapacity * sizeof(DictionaryEntry));
						if (pNewDictionary == nullptr) {
							free(pDictionary);
							DictEntries = 0;
							DictCapacity = 0;
							return;
						}
						else pDictionary = pNewDictionary;

						int* NewDPQ = (int*)realloc(DPQ, DictCapacity * sizeof(int));
						if (NewDPQ == nullptr) {
							free(DPQ);
							free(pDictionary);
							pDictionary = nullptr;
							DictEntries = 0;
							DictCapacity = 0;
							return;
						}
						else DPQ = NewDPQ;
					}
				}

				//  Add the new entry
				pDictionary[DictEntries].Offset = Offset;
				pDictionary[DictEntries].Hits = 1;
				pDictionary[DictEntries].Length = BYTE(Length);

				DPQ[DictEntries] = DictEntries;
				DictEntries++;

				//  Compute the number of bits needed to express the highest dictionary entry
				DictBits = 1;
				int BitCap = 2;

				while (BitCap < DictEntries) {
					DictBits++;
					BitCap = BitCap * 2;
				}

				//  Return to caller
				return;
			}

			//  showStatistics
			//
			//  Reports the statistics from the dictionary
			//
			//  PARAMETERS
			//
			//		ByteStrem&		-		Input or Output Byte Stream
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void	showStatistics(ByteStream& bsIn) {
				double		Ratio = 0.0;
				BYTE* pBuffer = bsIn.getBufferAddress();
				size_t		StrOff = 0;
				size_t		StrLen = 0;
				BYTE		c;

				os << "INFO: The dictionary contains: " << DictEntries << " entries with capacity for: " << DictCapacity << "." << std::endl;
				os << "INFO: The top one hundred entries (by frequency of reference) follows:-" << std::endl;

				//  Dump the top 20 entries from the priority queue
				for (int dpIndex = 0; dpIndex < 100 && dpIndex < DictEntries; dpIndex++) {
					StrOff = pDictionary[DPQ[dpIndex]].Offset;
					StrLen = pDictionary[DPQ[dpIndex]].Length;
					os << " " << (dpIndex + 1) << ". Ref. No: " << DPQ[dpIndex] << ", Offset: " << pDictionary[DPQ[dpIndex]].Offset << ", Length: " << int(pDictionary[DPQ[dpIndex]].Length) << ", References: " << pDictionary[DPQ[dpIndex]].Hits << " - '";
					for (size_t cIndex = 0; cIndex < StrLen; cIndex++) {
						c = pBuffer[StrOff + cIndex];
						if (c < 32) c = '.';
						os << c;
					}
					os << "'." << std::endl;
				}

				//  Show the encoding/decoding stats
				if (EncDecs == 0) Ratio = 0.0;
				else Ratio = double(TotalBits) / double(EncDecs);
				os << "INFO: " << EncDecs << " references were encoded/decoded in: " << TotalBits << " bits, " << Ratio << " bits per reference." << std::endl;

				os << "INFO: Native Codes: ";
				for (int sIndex = 0; sIndex < 8; sIndex++) {
					if (sIndex > 0) os << ", ";
					os << (sIndex + 1) << " - " << NDicBits[sIndex];
				}
				os << "." << std::endl;

				os << "                    ";
				for (int sIndex = 8; sIndex < 16; sIndex++) {
					if (sIndex > 8) os << ", ";
					os << (sIndex + 1) << " - " << NDicBits[sIndex];
				}
				os << "." << std::endl;

				os << "INFO: Fixed length extended codes: " << FixedEncs << "." << std::endl;

				os << "INFO: Extended Codes: ";
				for (int sIndex = 0; sIndex < 8; sIndex++) {
					if (sIndex > 0) os << ", ";
					os << (sIndex + 1) << " - " << EDicBits[sIndex];
				}
				os << "." << std::endl;

				os << "                      ";
				for (int sIndex = 8; sIndex < 16; sIndex++) {
					if (sIndex > 8) os << ", ";
					os << (sIndex + 1) << " - " << EDicBits[sIndex];
				}
				os << "." << std::endl;

				//  Return to caller
				return;
			}

			//  getEntries
			//
			//  Returns the count of entries in the dictioary
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			size_t	getEntries() { return DictEntries; }

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Constants                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			static const size_t		DictPageSize = 1024;												//  Dictionary page size (entries)

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Nested Structures                                                                                     *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			typedef struct DictionaryEntry {
				size_t			Offset;
				size_t			Hits;
				BYTE			Length;
			} DictionaryEntry;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Members                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Dictionary
			DictionaryEntry* pDictionary;																//  Pointer to the dictionary table
			int				DictEntries;																//  Number of dictionary entries
			int				DictCapacity;																//  Dictionary capacity
			int				DictBits;																	//  Number of bits to express a dictionary entry
			int*			DPQ;																		//  Dictionary priority quewue

			//  Statistics
			size_t			EncDecs;																	//  Number of encodes or decodes
			size_t			TotalBits;																	//  Total bits encoded or decoded
			size_t			NDicBits[16];																//  Native dicbit length encodes
			size_t			FixedEncs;																	//  Fixed size encodings
			size_t			EDicBits[16];																//  Extended DicBits encodes

			std::ostream& os;																			//  Stream for reporting

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************


		};

	};

}
