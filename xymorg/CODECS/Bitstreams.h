#pragma once
//
//*******************************************************************************************************************
//*																													*
//*   File:       Bitstreams.h																						*
//*   Suite:      xymorg integration																				*
//*   Version:    2.0.0	  Build:  02																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2016 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*	Bitstreams.h																									*
//*																													*
//*	This header file contains the class definition for the the bitstream classes.									*
//* The bitstream classes provide mechanisms for reading and writing arbitrary data as a stream of bit strings.		*
//* The header file also defines the bytestream classes that act as providers of a serial 8 bit wide stream for     *
//* the bitstreams.																									*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 26/09/2016   -  Initial version																			*
//*	2.0.0 - 10/02/2018   -  xymorg integration																		*
//*																													*
//*******************************************************************************************************************

//  Platform/Language Headers
#include	"../LPBHdrs.h"

//  Application Headers
#include	"../types.h"
#include	"../consts.h"

//  xymorg namespace
namespace xymorg {

	//*******************************************************************************************************************
	//*																													*
	//*   ByteStream Class 																								*
	//*																													*
	//*		The ByteStream class virtualises a memory buffer as a serial stream, the class provides the base			*
	//*		memory buffer support for the BitStream classes. This is also the base class for specialisations that		*
	//*     implement specialised memory buffer formats and behaviours.													*
	//*																													*
	//*******************************************************************************************************************

	class ByteStream {
	public:

		//*******************************************************************************************************************
		//*																													*
		//*   Constructors 																									*
		//*																													*
		//*******************************************************************************************************************

		//  Default Constructor
		//
		//  Constructs a default ByteStream object that has no underlying memory buffer and is thus not conditioned to
		//  support reading or writing.
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//	NOTES
		//

		ByteStream() {

			//  Set the object to the base state (no buffer)
			setNullState();

			//  Return to caller
			return;
		}

		//  Constructor		-	Supporting an extensible output buffer	
		//
		//  Constructs a ByteStream object that has an extensible output buffer that is owned by the stream.
		//
		//  PARAMETERS
		//
		//		size_t			-		Initial allocation size of the buffer (Bytes)
		//		size_t			-		Incremental allocation size (Bytes)
		//
		//	RETURNS
		//
		//	NOTES
		//

		ByteStream(size_t InitAllocSize, size_t IncAllocSize) {
			//  Set the object to the base state (no buffer)
			setNullState();

			//  Adjust initial allocation
			if (InitAllocSize < 256) InitAllocSize = 256;
			if (IncAllocSize != 0 && IncAllocSize < 256) IncAllocSize = 256;

			//  Allocate the initial buffer to the specified size
			Buffer = (BYTE*)malloc(InitAllocSize);
			if (Buffer == nullptr) return;

			//  Set the state of the ByteStream positioned for writing
			BufferSize = InitAllocSize;
			BufferInc = IncAllocSize;
			IsOwned = true;

			if (BufferSize > 0) EndOfStream = false;

			//  Return to caller
			return;
		}

		//  Constructor		-	Supporting a non-extensible output buffer	
		//
		//  Constructs a ByteStream object that has a non-extensible output buffer that is owned by the stream.
		//
		//  PARAMETERS
		//
		//		size_t			-		Initial allocation size of the buffer (Bytes)
		//
		//	RETURNS
		//
		//	NOTES
		//
		//		1.		Buufer overruns result in the additional bytes written being discarded.
		//

		ByteStream(size_t InitAllocSize) {
			//  Set the object to the base state (no buffer)
			setNullState();

			//  Allocate the initial buffer to the specified size
			Buffer = (BYTE*)malloc(InitAllocSize);
			if (Buffer == nullptr) return;

			//  Set the state of the ByteStream positioned for writing
			BufferSize = InitAllocSize;
			BufferInc = 0;
			IsOwned = true;

			if (BufferSize > 0) EndOfStream = false;

			//  Return to caller
			return;
		}

		//  Constructor		-	Supporting an input buffer
		//
		//  Constructs a ByteStream object that is based on existing content in a passed buffer owned by the caller
		//
		//  PARAMETERS
		//
		//		BYTE *			-		Pointer to the buffer containg the stream content
		//		size_t			-		Size of the buffer (Bytes)
		//
		//	RETURNS
		//
		//	NOTES
		//

		ByteStream(BYTE* pNewBuffer, size_t NewBfrSize) {

			//  Set the object to the base state (no buffer)
			setNullState();

			//  Setup the external buffer
			Buffer = pNewBuffer;
			BufferSize = NewBfrSize;
			BufferInc = 0;
			IsOwned = false;

			//  If we have input then clear the End-Of-Stream indicator
			if (BufferSize > 0) EndOfStream = false;

			//  Return to caller
			return;
		}

		//  Constructor		-	Copy Constructor
		//
		//  Constructs a ByteStream object from an existing ByteStream, the copy inherits the underlying buffer (if any) BUT ownership
		//  remains with the source ByteStream. Position within the stream is set to the beginning.
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Const reference to the source bytestream
		//
		//	RETURNS
		//
		//	NOTES
		//

		ByteStream(const ByteStream& Src) {

			//  Set the object to the base state (no buffer)
			setNullState();

			//  Copy the buffer attributes
			Buffer = Src.Buffer;
			BufferSize = Src.BufferSize;
			BufferInc = Src.BufferInc;
			IsOwned = false;

			if (BufferSize > 0) EndOfStream = false;

			//  Return to caller
			return;
		}

		//  Constructor		-	Move Constructor
		//
		//  Constructs a ByteStream object from an existing ByteStream, the copy inherits the ownersip state of underlying buffer (if any).
		//  Position within the stream is set to the beginning.
		//
		//  PARAMETERS
		//
		//		ByteStream&&		-		Reference to the source bytestream
		//
		//	RETURNS
		//
		//	NOTES
		//

		ByteStream(ByteStream&& Src) noexcept {

			//  Set the object to the base state (no buffer)
			setNullState();

			//  Copy the buffer attributes
			Buffer = Src.Buffer;
			BufferSize = Src.BufferSize;
			BufferInc = Src.BufferInc;
			IsOwned = Src.IsOwned;

			if (BufferSize > 0) EndOfStream = false;

			//  Disown the buffer in the source ByteStream
			Src.IsOwned = false;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*																													*
		//*   Destructor 																									*
		//*																													*
		//*******************************************************************************************************************
		~ByteStream() {
			//  If there is an underlying buffer and it is owned by the ByteStream then destroy the buffer
			if (Buffer != nullptr && IsOwned) free(Buffer);
			Buffer = nullptr;

			//  Return to caller
			return;
		};

		//*******************************************************************************************************************
		//*																													*
		//*  Public Members																									*
		//*																													*
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*																													*
		//*  Public Functions																								*
		//*																													*
		//*******************************************************************************************************************

		//  Stream API

		//  eos
		//
		//  Tests if the input buffer is at the end-of-stream
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//		bool			-	true if the end-of-stream is asserted otherwise false
		//
		//	NOTES
		//

		bool	eos() { return EndOfStream; }

		//  next
		//
		//  Reads the next byte from the input buffer
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//		BYTE		-	The next byte available from the buffer, returns zero if no data is available
		//
		//	NOTES
		//

		virtual BYTE next() {

			//  If end-of-stream has already been detected return 0
			if (EndOfStream) return 0;

			//  Increment the count of bytes read and test for end-of-stream
			BytesRead++;
			if (BytesRead == BufferSize) EndOfStream = true;

			//  Return the next available byte
			return Buffer[BytesRead - 1];
		}

		//  next
		//
		//  Writes the next byte to the output buffer
		//
		//  PARAMETERS
		//
		//		BYTE		-	The next byte to be written to the output
		//
		//	RETURNS
		//
		//	NOTES
		//

		virtual void next(BYTE NextByte) {
			//  Check to see if the output buffer is full - if so dump the byte
			if (EndOfStream) return;

			//  Increment the count of bytes written and check for buffer full
			BytesWritten++;
			if (BytesWritten == BufferSize) {
				//  If the Buffer is extensible then extend it, otherwise signal End-Of-Stream
				if (BufferInc != 0) {
					BYTE* NewBuffer = (BYTE*)realloc(Buffer, BufferSize + BufferInc);
					if (NewBuffer != nullptr) {
						Buffer = NewBuffer;
						BufferSize += BufferInc;
					}
					else EndOfStream = true;
				}
				else EndOfStream = true;
			}

			//  Post the byte to the next position in the buffer
			Buffer[BytesWritten - 1] = NextByte;
			return;
		}

		//  advance
		//
		//  Advances the current read position by the specified number of bytes
		//
		//  PARAMETERS
		//
		//		size_t			-		Number of bytes to advance the stream
		//
		//	RETURNS
		//
		//	NOTES
		//

		virtual void advance(size_t Distance) {

			//  Increment the position in the buffer
			BytesRead += Distance;

			//  Check for End-Of-Stream
			if (BytesRead >= BufferSize) {
				BytesRead = BufferSize;
				EndOfStream = true;
			}

			//  Return to caller
			return;
		}

		//  retreat
		//
		//  Retreats the current read position by the specified number of bytes
		//
		//  PARAMETERS
		//
		//		size_t			-		Number of bytes to retreat the stream
		//
		//	RETURNS
		//
		//	NOTES
		//

		virtual void retreat(size_t Distance) {

			//  If End-Of-Stream is set then unset if
			EndOfStream = false;

			//  Check that the stream can be retreated by the distance specified
			if (BytesRead > Distance) BytesRead = BytesRead - Distance;
			else BytesRead = 0;

			//  Return to caller
			return;
		}

		//  peek
		//
		//  Returns the character at the given offset from the current position in the read buffer.
		//  The current position is unaffected.
		//
		//  PARAMETERS
		//
		//		size_t			-		Number of bytes offset from the current position
		//
		//	RETURNS
		//
		//		BYTE			-		Character at the indicated position
		//
		//	NOTES
		//

		virtual BYTE peek(size_t Offset) {
			//  Overrun will return 0
			if (BytesRead + Offset >= BufferSize) return 0;
			return Buffer[BytesRead + Offset];
		}

		//  flush
		//
		//  For a writeable stream this function performs any post-writing updates to the stream content.
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//	NOTES
		//

		virtual void flush() {
			//  Return to caller
			return;
		}

		//  Buffer management functions

		//  Getters & Setters for members

		size_t		getBytesWritten() { return BytesWritten; }
		size_t		getBytesRead() { return BytesRead; }
		void		setSize(size_t NewSize) { BufferSize = NewSize; return; }

		//  Assignment Operator Overloads

		//  Copy Assignment
		//
		//  The copy inherits the underlying buffer (if any) BUT ownership
		//  remains with the source ByteStream. Position within the stream is set to the beginning.
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the source bytestream
		//
		//	RETURNS
		//
		//		ByteStream&		-		Reference to the target (self) bytestream
		//
		//	NOTES
		//

		ByteStream& operator = (const ByteStream& Src) {

			//  If this ByteStream already owns a buffer then free it
			if (Buffer != NULL && IsOwned) free(Buffer);

			//  Set the object to the base state
			setNullState();

			//  Copy the buffer attributes
			Buffer = Src.Buffer;
			BufferSize = Src.BufferSize;
			BufferInc = Src.BufferInc;
			IsOwned = false;

			if (BufferSize > 0) EndOfStream = false;

			//  Return self reference
			return *this;
		}

		//  Move Assignment
		//
		//  The copy inherits ownership state of the underlying buffer (if any) 
		//  Position within the stream is set to the beginning.
		//
		//  PARAMETERS
		//
		//		ByteStream&&	-		Reference to the source bytestream
		//
		//	RETURNS
		//
		//		ByteStream&		-		Reference to the target (self) bytestream
		//
		//	NOTES
		//

		ByteStream& operator = (ByteStream&& Src) noexcept {

			//  If this ByteStream already owns a buffer then free it
			if (Buffer != nullptr && IsOwned) free(Buffer);

			//  Set the object to the base state
			setNullState();

			//  Copy the buffer attributes
			Buffer = Src.Buffer;
			BufferSize = Src.BufferSize;
			BufferInc = Src.BufferInc;
			IsOwned = Src.IsOwned;

			if (BufferSize > 0) EndOfStream = false;

			//  Disown the buffer in the source ByteStream
			Src.IsOwned = false;

			//  Return self reference
			return *this;
		}

		//
		//  Buffer and Offset Getters
		//  =========================
		//

		BYTE* getBufferAddress() { return Buffer; }
		BYTE* getReadAddress() { return Buffer + BytesRead; }
		BYTE* getWriteAddress() { return Buffer + BytesWritten; }
		BYTE* getPreReadWindow(size_t DesiredWindow, size_t& ActualWindows) {
			ActualWindows = 0;
			if (Buffer == nullptr) return nullptr;
			if (DesiredWindow > BytesRead) {
				ActualWindows = BytesRead;
				return Buffer;
			}
			ActualWindows = DesiredWindow;
			return Buffer + (BytesRead - DesiredWindow);
		}

		size_t		getRemainder() { if (Buffer == nullptr) return 0; return BufferSize - BytesRead; }

		//  acquireBuffer
		//
		//  Acquires ownership of the underlying buffer.
		//
		//  PARAMETERS
		//
		//		size_t&			-		Reference to the variable where the buffer size will be returned
		//
		//	RETURNS
		//
		//		BYTE*			-		Pointer to the underlying buffer
		//
		//	NOTES
		//

		BYTE* acquireBuffer(size_t& BSize) {

			//  If the buffer is not owned then nothing can be done
			if (!IsOwned) {
				BSize = 0;
				return nullptr;
			}

			//  Remove ownership
			IsOwned = false;

			//  Return the buffer
			BSize = BytesWritten;
			return Buffer;
		}

	protected:

		//*******************************************************************************************************************
		//*																													*
		//*  Protected Members																								*
		//*																													*
		//*******************************************************************************************************************

		//  Buffer characteristics
		BYTE*				Buffer;																		//  Base buffer address
		bool				IsOwned;																	//  Buffer is owned by this ByteStream
		size_t				BufferSize;																	//  Current buffer size (bytes)
		size_t				BufferInc;																	//  Buffer allocation increment size (bytes)
		bool				EndOfStream;																//  End of stream has been read

		//  Dynamic positions in the buffer
		size_t				BytesRead;																	//  Number of bytes consumed
		size_t				BytesWritten;																//  Number of bytes written

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Functions																								*
		//*																													*
		//*******************************************************************************************************************

		//  setNullState
		//
		//  Sets all members of the ByteStream to their base state, reflecting a stream with no underlying buffer.
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//	NOTES
		//

		void setNullState() {
			Buffer = nullptr;
			IsOwned = false;
			EndOfStream = true;
			BytesRead = 0;
			BytesWritten = 0;

			//  Return to caller
			return;
		}

	};

	//*******************************************************************************************************************
	//*																													*
	//*   SegmentedStream Class 																						*
	//*																													*
	//*		The SegmentedStream class virtualises a memory buffer as a serial stream, the class provides the base 		*
	//*		memory buffer support for the Bitstream classes.															*
	//*		The call extends the ByteStream base class to provide the interface needed by BitStreams.					*
	//*		The storage model splits the stream into a series of up to 256 byte segments each segment carries the 		*
	//*		length of data in the segment in the first byte of the segment.												*
	//*																													*
	//*		This storage model supports the storage for GIF Images.														*
	//*																													*
	//*																													*
	//*******************************************************************************************************************

	class SegmentedStream : public ByteStream {
	public:

		//*******************************************************************************************************************
		//*																													*
		//*   Constructors 																									*
		//*																													*
		//*******************************************************************************************************************

		//  Default Constructor
		//
		//  Constructs a default SegmentedStream object that has no underlying memory buffer and is thus not conditioned to
		//  support reading or writing.
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//	NOTES
		//

		SegmentedStream() : ByteStream() {

			SegLen = 0;

			//  Return to caller
			return;
		}

		//  Constructor		-	Supporting an extensible output buffer	
		//
		//  Constructs a SegmentedStream object that has an extensible output buffer that is owned by the stream.
		//
		//  PARAMETERS
		//
		//		size_t			-		Initial allocation size of the buffer (Bytes)
		//		size_t			-		Incremental allocation size (Bytes)
		//
		//	RETURNS
		//
		//	NOTES
		//

		SegmentedStream(size_t InitAllocSize, size_t IncAllocSize) : ByteStream(InitAllocSize, IncAllocSize) {

			SegLen = 0;

			if (Buffer != nullptr) {
				*Buffer = 0xFF;														//  255 byte length segment
				BytesWritten++;
			}

			//  Return to caller
			return;
		}

		//  Constructor		-	Supporting a non-extensible output buffer	
		//
		//  Constructs a SegmentedStream object that has a non-extensible output buffer that is owned by the stream.
		//
		//  PARAMETERS
		//
		//		size_t			-		Initial allocation size of the buffer (Bytes)
		//
		//	RETURNS
		//
		//	NOTES
		//
		//		1.		Buufer overruns result in the additional bytes written being discarded.
		//

		SegmentedStream(size_t InitAllocSize) : ByteStream(InitAllocSize) {

			SegLen = 0;

			if (Buffer != nullptr) {
				*Buffer = 0xFF;														//  255 byte length segment
				BytesWritten++;
			}

			//  Return to caller
			return;
		}

		//  Constructor		-	Supporting an input buffer
		//
		//  Constructs a SegmentedStream object that is based on existing content in a passed buffer owned by the caller.
		//
		//  PARAMETERS
		//
		//		BYTE *			-		Pointer to the buffer containg the stream content
		//		size_t			-		Size of the buffer (Bytes)
		//
		//	RETURNS
		//
		//	NOTES
		//

		SegmentedStream(BYTE* pNewBuffer, size_t NewBfrSize) : ByteStream(pNewBuffer, NewBfrSize) {

			SegLen = 0;

			//  If there is no buffered data available show end-of-stream
			if (Buffer == nullptr || BufferSize <= 1) {
				BytesRead = BufferSize;
				EndOfStream = true;
				return;
			}

			//  Capture the length of data available in the first segment
			SegLen = *Buffer;
			if (SegLen == 0) {
				BytesRead = 1;
				EndOfStream = true;
				return;
			}

			//  Condition for reading the initial segment of data
			BytesRead = 1;

			//  Return to caller
			return;
		}

		//  Constructor		-	Copy Constructor
		//
		//  Constructs a SegmentedStream object from an existing SegmentedStream, the copy inherits the underlying buffer (if any) BUT ownership
		//  remains with the source ByteStream. Position within the stream is set to the beginning.
		//
		//  PARAMETERS
		//
		//		SegmentedStream&		-		Const reference to the source SegmentedStream
		//
		//	RETURNS
		//
		//	NOTES
		//

		SegmentedStream(const SegmentedStream& Src) : ByteStream(Src) {

			SegLen = 0;

			//  If there is no buffered data available show end-of-stream
			if (Buffer == nullptr || BufferSize <= 1) {
				BytesRead = BufferSize;
				EndOfStream = true;
				return;
			}

			//  Capture the length of data available in the first segment
			SegLen = *Buffer;
			if (SegLen == 0) {
				BytesRead = 1;
				EndOfStream = true;
				return;
			}

			//  Condition for reading the initial segment of data
			BytesRead = 1;

			//  Return to caller
			return;
		}

		//  Constructor		-	Move Constructor
		//
		//  Constructs a SegmentedStream object from an existing SegmentedStream, the copy inherits the ownersip state of underlying buffer (if any).
		//  Position within the stream is set to the beginning.
		//
		//  PARAMETERS
		//
		//		SegmentedStream&&		-		Reference to the source SegmentedStream
		//
		//	RETURNS
		//
		//	NOTES
		//

		SegmentedStream(SegmentedStream&& Src) noexcept : ByteStream(Src) {

			SegLen = 0;

			//  If there is no buffered data available show end-of-stream
			if (Buffer == nullptr || BufferSize <= 1) {
				EndOfStream = true;
				BytesRead = BufferSize;
				return;
			}

			//  Capture the length of data available in the first segment
			SegLen = *Buffer;
			if (SegLen == 0) {
				EndOfStream = true;
				BytesRead = 1;
				return;
			}

			//  Condition for reading the initial segment of data
			BytesRead = 1;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*																													*
		//*   Destructor 																									*
		//*																													*
		//*******************************************************************************************************************

		~SegmentedStream() {

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*																													*
		//*  Public Members																									*
		//*																													*
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*																													*
		//*  Public Functions																								*
		//*																													*
		//*******************************************************************************************************************

		//  next
		//
		//  Reads the next byte from the input buffer, if a segment is exhausted then the length of the next segment
		//	is loaded and skipped over.
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//		BYTE		-	The next byte available from the buffer, returns zero if no data is available
		//
		//	NOTES
		//

		virtual BYTE next() {

			//  If end-of-stream has already been detected return 0
			if (EndOfStream) return 0;

			//  Test for exhaustion of the current segment
			if (SegLen == 0) {
				//  Advance to the next segment - if one is available
				SegLen = Buffer[BytesRead];
				BytesRead++;
				if (SegLen == 0) {
					EndOfStream = true;
					return 0;
				}
				if (BytesRead == BufferSize) {
					EndOfStream = true;
					return 0;
				}
			}

			//  Increment the count of bytes read and test for end-of-stream
			BytesRead++;
			SegLen--;
			if (BytesRead == BufferSize) EndOfStream = true;
			if (SegLen == 0 && Buffer[BytesRead] == 0) EndOfStream = true;

			//  Return the next available byte
			return Buffer[BytesRead - 1];
		}

		//  next
		//
		//  Writes the next byte to the output buffer, if a 255 byte buffer segment has been filled then a new
		//  255 byte segment is started. The actual content length of the final segment will be adjusted by
		//  the 'flush' processing.
		//
		//  PARAMETERS
		//
		//		BYTE		-	The next byte to be written to the output
		//
		//	RETURNS
		//
		//	NOTES
		//

		virtual void next(BYTE NextByte) {
			//  Check to see if the output buffer is full - if so dump the byte
			if (EndOfStream) return;

			//  Increment the count of bytes written and check for buffer full
			BytesWritten++;
			if (BytesWritten == BufferSize) {
				//  If the Buffer is extensible then extend it, otherwise signal End-Of-Stream
				if (BufferInc != 0) {
					BYTE* NewBuffer = (BYTE*)realloc(Buffer, BufferSize + BufferInc);
					if (NewBuffer != nullptr) {
						Buffer = NewBuffer;
						BufferSize += BufferInc;
					}
					else EndOfStream = true;
				}
				else EndOfStream = true;
			}

			//  Post the byte to the next position in the buffer
			Buffer[BytesWritten - 1] = NextByte;
			SegLen++;

			//  Check for a full segment (255 bytes) if so start a new 255 byte segment
			if (SegLen == 255) {
				if ((BytesWritten + 2) >= BufferSize) {
					EndOfStream = true;
					Buffer[BytesWritten] = 0x00;
					BytesWritten++;
					return;
				}
				Buffer[BytesWritten] = 0xFF;
				BytesWritten++;
				SegLen = 0;
			}

			return;
		}

		//
		//  advance, retreat and peek api is NOT supported on a segmented stream
		//

		virtual void advance(size_t Distance) { Distance = Distance;  return; }
		virtual void retreat(size_t Distance) { Distance = Distance;  return; }
		virtual BYTE peek(size_t Offset) { Offset = Offset;  return 0; }

		//  flush
		//
		//  For a writeable stream this function performs any post-writing updates to the stream content.
		//  For a segmented stream it will update the length of the last segment and append a zero length
		//	segment to the stream.
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//	NOTES
		//

		virtual void flush() {
			BYTE* pLS = nullptr;													//  Pointer to the last segment

			pLS = (Buffer + BytesWritten) - (SegLen + 1);								//  Last segment size byte

			//  Update the length
			*pLS = BYTE(SegLen);
			if (SegLen > 0) {
				Buffer[BytesWritten] = 0x00;
				BytesWritten++;
			}

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		size_t			SegLen;													//  Length of data available in the current segment

	};

	//*******************************************************************************************************************
	//*																													*
	//*   StuffedStream Class 																							*
	//*																													*
	//*		The StuffedByteStream class virtualises a memory buffer as a serial stream, the class provides the base		*
	//*		memory buffer support for the BitStream classes. The class detects byte stuffing used in JPEG encoding,		*
	//*		it will drop any 0x00 bytes in the stream that immediately follow a 0xFF byte.								*
	//*		Detection of 0xFF followed by non-0x00 byte will be treated as a JPEG marker and will signal EOS on			*
	//*		the stream;																									*
	//*		For output streams a 0xFF BYTE the writer will insert the stuff 0x00 BYTE in the stream.					*
	//*																													*
	//*		This storage model supports the storage for JPEG Images.													*
	//*																													*
	//*																													*
	//*******************************************************************************************************************

	class StuffedStream : public ByteStream {
	public:

		//*******************************************************************************************************************
		//*																													*
		//*   Constructors 																									*
		//*																													*
		//*******************************************************************************************************************

		//  Default Constructor
		//
		//  Constructs a default StuffedStream object that has no underlying memory buffer and is thus not conditioned to
		//  support reading or writing.
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//	NOTES
		//

		StuffedStream() : ByteStream() {

			//  Return to caller
			return;
		}

		//  Constructor		-	Supporting an extensible output buffer	
		//
		//  Constructs a StuffedStream object that has an extensible output buffer that is owned by the stream.
		//
		//  PARAMETERS
		//
		//		size_t			-		Initial allocation size of the buffer (Bytes)
		//		size_t			-		Incremental allocation size (Bytes)
		//
		//	RETURNS
		//
		//	NOTES
		//

		StuffedStream(size_t InitAllocSize, size_t IncAllocSize) : ByteStream(InitAllocSize, IncAllocSize) {

			//  Return to caller
			return;
		}

		//  Constructor		-	Supporting a non-extensible output buffer	
		//
		//  Constructs a StuffedStream object that has a non-extensible output buffer that is owned by the stream.
		//
		//  PARAMETERS
		//
		//		size_t			-		Initial allocation size of the buffer (Bytes)
		//
		//	RETURNS
		//
		//	NOTES
		//
		//		1.		Buufer overruns result in the additional bytes written being discarded.
		//

		StuffedStream(size_t InitAllocSize) : ByteStream(InitAllocSize) {

			//  Return to caller
			return;
		}

		//  Constructor		-	Supporting an input buffer
		//
		//  Constructs a StuffedStream object that is based on existing content in a passed buffer owned by the caller.
		//
		//  PARAMETERS
		//
		//		BYTE *			-		Pointer to the buffer containg the stream content
		//		size_t			-		Size of the buffer (Bytes)
		//
		//	RETURNS
		//
		//	NOTES
		//

		StuffedStream(BYTE* pNewBuffer, size_t NewBfrSize) : ByteStream(pNewBuffer, NewBfrSize) {

			//  Return to caller
			return;
		}

		//  Constructor		-	Copy Constructor
		//
		//  Constructs a StuffedStream object from an existing StuffedStream, the copy inherits the underlying buffer (if any) BUT ownership
		//  remains with the source ByteStream. Position within the stream is set to the beginning.
		//
		//  PARAMETERS
		//
		//		StuffedStream&		-		Const reference to the source StuffedStream
		//
		//	RETURNS
		//
		//	NOTES
		//

		StuffedStream(const StuffedStream& Src) : ByteStream(Src) {

			//  Return to caller
			return;
		}

		//  Constructor		-	Move Constructor
		//
		//  Constructs a StuffedStream object from an existing StuffedStream, the copy inherits the ownersip state of underlying buffer (if any).
		//  Position within the stream is set to the beginning.
		//
		//  PARAMETERS
		//
		//		StuffedStream&&		-		Reference to the source StuffedStream
		//
		//	RETURNS
		//
		//	NOTES
		//

		StuffedStream(StuffedStream&& Src) noexcept : ByteStream(Src) {

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*																													*
		//*   Destructor 																									*
		//*																													*
		//*******************************************************************************************************************

		~StuffedStream() {

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*																													*
		//*  Public Members																									*
		//*																													*
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*																													*
		//*  Public Functions																								*
		//*																													*
		//*******************************************************************************************************************

		//  next
		//
		//  Reads the next byte from the input buffer, destuffing the input.
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//		BYTE		-	The next byte available from the buffer, returns zero if no data is available
		//
		//	NOTES
		//

		virtual BYTE next() {

			//  If end-of-stream has already been detected return 0
			if (EndOfStream) return 0;

			//  Increment the count of bytes read and test for end-of-stream
			BytesRead++;
			if (BytesRead == BufferSize) EndOfStream = true;

			//  Return the next available byte
			if (Buffer == nullptr) return 0;

			//  Test for BYTE stuffing or JPEG marker segments  must also skip restart markers
			if ((Buffer[BytesRead - 1] == 0xFF) && ((Buffer[BytesRead] & 0xF8) == 0xD0)) BytesRead += 2;
			if (Buffer[BytesRead - 1] == 0xFF)
			{
				if (Buffer[BytesRead] != 0x00)
				{
					EndOfStream = true;
					return 0;
				}
			}
			else
			{
				if (Buffer[BytesRead - 1] == 0x00)
				{
					if (Buffer[BytesRead - 2] == 0xFF)
					{
						//  Skipping stuffed BYTE
						BytesRead++;
						if (BytesRead == BufferSize) EndOfStream = true;
					}
				}
			}

			return Buffer[BytesRead - 1];
		}

		//  next
		//
		//  Writes the next byte to the output buffer, stuffing the output.
		//
		//  PARAMETERS
		//
		//		BYTE		-	The next byte to be written to the output
		//
		//	RETURNS
		//
		//	NOTES
		//

		virtual void next(BYTE NextByte) {
			//  Check to see if the output buffer is full - if so dump the byte
			if (EndOfStream) return;

			//  Increment the count of bytes written and check for buffer full
			BytesWritten++;
			if (BytesWritten == BufferSize) EndOfStream = true;

			//  Check if this is a NULL buffer
			if (Buffer == nullptr) return;

			//  Post the byte to the next position in the buffer
			Buffer[BytesWritten - 1] = NextByte;

			//  if the byte written was 0xFF then stuff a 0x00 to follow
			if (NextByte == 0xFF)
			{
				if (EndOfStream) return;
				//  Increment the count of bytes written and check for buffer full
				BytesWritten++;
				if (BytesWritten == BufferSize) EndOfStream = true;

				//  Post the stuffing byte to the next position in the buffer
				Buffer[BytesWritten - 1] = 0x00;
			}

			return;
		}

		//
		//  advance, retreat and peek api is NOT supported on a stuffed stream
		//

		virtual void advance(size_t Distance) { Distance = Distance;  return; }
		virtual void retreat(size_t Distance) { Distance = Distance;  return; }
		virtual BYTE peek(size_t Offset) { Offset = Offset;  return 0; }

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

	};

	//*******************************************************************************************************************
	//*																													*
	//*   MSBitStream Class 																							*
	//*																													*
	//*		The MSBitStream class virtualises access to a stream of bytes treating it as a continuous stream of bits.	*
	//*		The bit order is MSB first to support various compression implementations of Huffman & Lempel-Ziv			*
	//*		algorithms.																									*
	//*		This implementation supports a maximum of 32 bits in an individual bit string.								*
	//*																													*
	//*******************************************************************************************************************

	class MSBitStream {
	public:

		//*******************************************************************************************************************
		//*																													*
		//*   Constructors 																									*
		//*																													*
		//*******************************************************************************************************************

		//  MSBitStream		-		Normal Constructor
		//
		//	Constructs a new MSBitStream object backed by the reference ByteStream.
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the ByteStream that provides the backing storage stream for the bit stream
		//		bool			-		If true the stream is conditined for writing otherwise it is conditioned for readiung
		//
		//	RETURNS
		//
		//	NOTES
		//

		MSBitStream(ByteStream& BackingStream, bool Writeable) : bsBase(BackingStream) {
			BitOffset = 0;
			EndOfStream = true;
			BufferedBits = 0;
			BitsWritten = 0;
			BitsRead = 0;

			//  Condition the stream for writing or for reading
			if (Writeable) {
				//  Condition for writing
				BitOffset = 0;
				BitsWritten = 0;
				EndOfStream = false;
				BufferedBits = 0;

				//  Clear the byte array
				ByteArray[0] = 0;
				ByteArray[1] = 0;
				ByteArray[2] = 0;
			}
			else {
				//  Condition for reading
				BitOffset = 0;
				BitsRead = 0;
				EndOfStream = true;
				BufferedBits = 0;

				//  Fill the byte array from the underlying byte stream
				if (!bsBase.eos())
				{
					EndOfStream = false;
					ByteArray[0] = bsBase.next();
					BufferedBits += 8;
				}
				else ByteArray[0] = 0;
				if (!bsBase.eos())
				{
					ByteArray[1] = bsBase.next();
					BufferedBits += 8;
				}
				else ByteArray[1] = 0;
				if (!bsBase.eos())
				{
					ByteArray[2] = bsBase.next();
					BufferedBits += 8;
				}
				else ByteArray[2] = 0;
			}

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*																													*
		//*   Destructor 																									*
		//*																													*
		//*******************************************************************************************************************

		~MSBitStream() {
			//  Return to caller
			return;
		};

		//*******************************************************************************************************************
		//*																													*
		//*  Public Members																									*
		//*																													*
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*																													*
		//*  Public Functions																								*
		//*																													*
		//*******************************************************************************************************************

		//  next
		//
		//  Reads the next bit string from the stream
		//
		//  PARAMETERS
		//
		//		uint32_t			-	Size of the bit string to read	
		// 
		//	RETURNS
		//
		//		uint32_t			-	String
		//
		//	NOTES
		//

		uint32_t next(uint32_t Bits)
		{
			uint32_t				Junior, Senior;												//  Junior and senior part of the string

			//  Envelope check
			if (Bits > 32) return 0;

			//  Read the parts of the string
			if (Bits > 16)
			{
				Senior = next(Bits - 16);
				Junior = next16(16);
			}
			else
			{
				Junior = next16(Bits);
				Senior = 0;
			}
			return (Senior << 16) | Junior;
		}

		//  next
		//
		//  Writes the next bit string to the stream
		//
		//  PARAMETERS
		//
		//		uint32_t			-	Bit string to be written
		//		size_t				-	Size of the bit string to write	
		// 
		//	RETURNS
		//
		//	NOTES
		//

		void next(uint32_t Out, uint32_t Bits)
		{
			if (Bits > 32) return;
			if (Bits > 16)
			{
				next16(static_cast<unsigned short>(Out >> 16), Bits - 16);
				next16(static_cast<unsigned short>(Out & 0x0000FFFF), 16);
			}
			else next16(static_cast<unsigned short>(Out), Bits);
			return;
		}

		//  flush
		//
		//  Forces any buffered bits to be written to the the current output stream
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//	NOTES
		//

		void flush() {

			//  Output any complete BYTES
			while (BitOffset > 0)
			{
				bsBase.next(ByteArray[0]);
				ByteArray[0] = ByteArray[1];
				ByteArray[1] = ByteArray[2];
				ByteArray[2] = 0;
				if (BitOffset < 8) BitOffset = 0;
				else BitOffset -= 8;
			}

			//  Signal flush on the underlying ByteStream
			bsBase.flush();

			return;
		}

		//  eos
		//
		//  Test for the end-of-stream condition being detected
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//		bool		-		true if the stream has been consumed/filled otherwise false
		//
		//	NOTES
		//

		bool eos()
		{
			if (bsBase.eos())
			{
				return EndOfStream;
			}
			return false;
		}

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		ByteStream&			bsBase;																//  Underlying Byte Stream
		uint32_t			BitOffset;															//  Offset of the next Bit to be read/written
		uint32_t			BitsRead;															//  Bits read (consumed) from the stream
		uint32_t			BitsWritten;														//  Bits written to the stream
		BYTE				ByteArray[3];														//  Array of bytes read from the underlying stream
		uint32_t			BufferedBits;														//  Count of buffered bits
		bool				EndOfStream;														//  End of stream indicator

		//*******************************************************************************************************************
		//*																													*
		//*  Private Functions																								*
		//*																													*
		//*******************************************************************************************************************

		//  next16
		//
		//  Reads the next bit string (up to 16 bits) from the stream
		//
		//  PARAMETERS
		//
		//		uint16_t			-	Size of the bit string to read	
		// 
		//	RETURNS
		//
		//		uint16_t			-	String
		//
		//	NOTES
		//

		uint16_t next16(uint32_t Bits)
		{
			uint32_t			Acc = 0;														//  Accumulator for the bit string
			uint32_t			Mask = 0xffffffff;												//  Mak for the Accumulator values

			//  Build the bitstring for the next n Bits
			Acc = ByteArray[0];
			Acc <<= 8;
			Acc |= ByteArray[1];
			Acc <<= 8;
			Acc |= ByteArray[2];

			Acc <<= BitOffset;
			Acc >>= (24 - Bits);

			Mask <<= (32 - Bits);
			Mask >>= (32 - Bits);
			Acc &= Mask;
			BitOffset += Bits;
			BitsRead += Bits;
			if (Bits > BufferedBits) BufferedBits = 0;
			else BufferedBits -= Bits;

			//  Replace any whole bytes that have been consumed
			while (BitOffset >= 8)
			{
				ByteArray[0] = ByteArray[1];
				ByteArray[1] = ByteArray[2];
				if (!bsBase.eos())
				{
					ByteArray[2] = bsBase.next();
					BufferedBits += 8;
				}
				else ByteArray[2] = 0;
				BitOffset -= 8;
			}

			//  Test for end of string
			if (bsBase.eos() && BufferedBits == 0) EndOfStream = true;

			//  Return the string
			return uint16_t(Acc);
		}

		//  next16
		//
		//  Writes the next bit string (up to 16 bits) to the stream
		//
		//  PARAMETERS
		//
		//		uint16_t			-   Value to be written
		//		size_t				-	Size of the bit string to read	
		// 
		//	RETURNS
		//
		//	NOTES
		//

		void next16(uint16_t Out, uint32_t Bits)
		{
			unsigned int			Acc = 0;									//  Bit string accumulator

			//  Load the existing BYTE array into the accumulator
			Acc = ByteArray[0];
			Acc <<= 8;
			Acc |= ByteArray[1];
			Acc <<= 8;
			Acc |= ByteArray[2];

			//  Accumulate the output bits
			Acc |= (Out << ((24 - BitOffset) - Bits));

			//  Store back the accumulated value
			ByteArray[2] = BYTE(Acc & 0x000000FF);
			ByteArray[1] = BYTE((Acc >> 8) & 0x000000FF);
			ByteArray[0] = BYTE((Acc >> 16) & 0x000000FF);

			BitOffset += Bits;
			BitsWritten += Bits;

			//  Output any complete BYTES
			while (BitOffset >= 8)
			{
				bsBase.next(ByteArray[0]);
				ByteArray[0] = ByteArray[1];
				ByteArray[1] = ByteArray[2];
				ByteArray[2] = 0;
				BitOffset -= 8;
			}

			//  Test for End-Of-Stream
			if (bsBase.eos()) EndOfStream = true;
			return;
		}
	};

	//*******************************************************************************************************************
	//*																													*
	//*   LSBitStream Class 																							*
	//*																													*
	//*		The LSBitStream class virtualises access to a stream of bytes treating it as a continuous stream of bits.	*
	//*		The bit order is LSB first to support compressed image formats such as GIF.									*
	//*		This implementation supports a maximum of 32 bits in an individual bit string.								*
	//*																													*
	//*******************************************************************************************************************

	class LSBitStream {
	public:

		//*******************************************************************************************************************
		//*																													*
		//*   Constructors 																									*
		//*																													*
		//*******************************************************************************************************************

		//  LSBitStream		-		Normal Constructor
		//
		//	Constructs a new MSBitStream object backed by the reference ByteStream.
		//
		//  PARAMETERS
		//
		//		ByteStream&		-		Reference to the ByteStream that provides the backing storage stream for the bit stream
		//		bool			-		If true the stream is conditined for writing otherwise it is conditioned for readiung
		//
		//	RETURNS
		//
		//	NOTES
		//

		LSBitStream(ByteStream& BackingStream, bool Writeable) : bsBase(BackingStream) {
			BitOffset = 0;
			EndOfStream = true;
			BufferedBits = 0;
			BitsWritten = 0;
			BitsRead = 0;

			//  Condition the stream for writing or for reading
			if (Writeable) {
				//  Condition for writing
				BitOffset = 0;
				BitsWritten = 0;
				EndOfStream = false;
				BufferedBits = 0;

				//  Clear the byte array
				ByteArray[0] = 0;
				ByteArray[1] = 0;
				ByteArray[2] = 0;
			}
			else {
				//  Condition for reading
				BitOffset = 0;
				BitsRead = 0;
				EndOfStream = true;
				BufferedBits = 0;

				//  Fill the byte array from the underlying byte stream
				if (!bsBase.eos())
				{
					EndOfStream = false;
					ByteArray[0] = bsBase.next();
					BufferedBits += 8;
				}
				else ByteArray[0] = 0;
				if (!bsBase.eos())
				{
					ByteArray[1] = bsBase.next();
					BufferedBits += 8;
				}
				else ByteArray[1] = 0;
				if (!bsBase.eos())
				{
					ByteArray[2] = bsBase.next();
					BufferedBits += 8;
				}
				else ByteArray[2] = 0;
			}

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*																													*
		//*   Destructor 																									*
		//*																													*
		//*******************************************************************************************************************

		~LSBitStream() {
			//  Return to caller
			return;
		};

		//*******************************************************************************************************************
		//*																													*
		//*  Public Members																									*
		//*																													*
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*																													*
		//*  Public Functions																								*
		//*																													*
		//*******************************************************************************************************************

		//  next
		//
		//  Reads the next bit string from the stream
		//
		//  PARAMETERS
		//
		//		uint32_t			-	Size of the bit string to read	
		// 
		//	RETURNS
		//
		//		uint32_t			-	String
		//
		//	NOTES
		//

		uint32_t next(uint32_t Bits)
		{
			uint32_t				Junior, Senior;												//  Junior and senior part of the string

			//  Envelope check
			if (Bits > 32) return 0;

			//  Read the parts of the string
			if (Bits > 16)
			{
				Junior = next16(16);
				Senior = next(Bits - 16);
			}
			else
			{
				Junior = next16(Bits);
				Senior = 0;
			}
			return (Senior << 16) | Junior;
		}

		//  next
		//
		//  Writes the next bit string to the stream
		//
		//  PARAMETERS
		//
		//		uint32_t			-	Bit string to be written
		//		uint32_t			-	Size of the bit string to write	
		// 
		//	RETURNS
		//
		//	NOTES
		//

		void next(uint32_t Out, uint32_t Bits)
		{
			if (Bits > 32) return;
			if (Bits > 16)
			{
				next16(static_cast<unsigned short>(Out & 0x0000FFFF), 16);
				next16(static_cast<unsigned short>(Out >> 16), Bits - 16);
			}
			else next16(static_cast<unsigned short>(Out), Bits);
			return;
		}

		//  flush
		//
		//  Forces any buffered bits to be written to the the current output stream
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//	NOTES
		//

		void flush() {

			//  Output any complete BYTES
			while (BitOffset > 0)
			{
				bsBase.next(ByteArray[2]);
				ByteArray[2] = ByteArray[1];
				ByteArray[1] = ByteArray[0];
				ByteArray[0] = 0;
				if (BitOffset < 8) BitOffset = 0;
				else BitOffset -= 8;
			}

			//  Signal flush on the underlying ByteStream
			bsBase.flush();

			return;
		}

		//  eos
		//
		//  Test for the end-of-stream condition being detected
		//
		//  PARAMETERS
		//
		//	RETURNS
		//
		//		bool		-		true if the stream has been consumed/filled otherwise false
		//
		//	NOTES
		//

		bool eos()
		{
			if (bsBase.eos())
			{
				return EndOfStream;
			}
			return false;
		}

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		ByteStream&			bsBase;																//  Underlying Byte Stream
		uint32_t			BitOffset;															//  Offset of the next Bit to be read/written
		uint32_t			BitsRead;															//  Bits read (consumed) from the stream
		uint32_t			BitsWritten;														//  Bits written to the stream
		BYTE				ByteArray[3];														//  Array of bytes read from the underlying stream
		uint32_t			BufferedBits;														//  Count of buffered bits
		bool				EndOfStream;														//  End of stream indicator

		//*******************************************************************************************************************
		//*																													*
		//*  Private Functions																								*
		//*																													*
		//*******************************************************************************************************************

		//  next16
		//
		//  Reads the next bit string (up to 16 bits) from the stream
		//
		//  PARAMETERS
		//
		//		uint16_t			-	Size of the bit string to read	
		// 
		//	RETURNS
		//
		//		uint16_t			-	String
		//
		//	NOTES
		//

		uint16_t next16(uint32_t Bits)
		{
			uint32_t			Acc = 0;														//  Accumulator for the bit string

			//  Build the bitstring for the next n Bits

			Acc = ByteArray[2];
			Acc <<= 8;
			Acc |= ByteArray[1];
			Acc <<= 8;
			Acc |= ByteArray[0];
			Acc >>= BitOffset;
			Acc <<= (24 - Bits);
			Acc &= 0x00FFFFFF;
			Acc >>= (24 - Bits);

			BitOffset += Bits;
			BitsRead += Bits;
			if (Bits > BufferedBits) BufferedBits = 0;
			else BufferedBits -= Bits;

			//  Replace any whole bytes that have been consumed
			while (BitOffset >= 8)
			{
				ByteArray[0] = ByteArray[1];
				ByteArray[1] = ByteArray[2];
				if (!bsBase.eos())
				{
					ByteArray[2] = bsBase.next();
					BufferedBits += 8;
				}
				else ByteArray[2] = 0;
				BitOffset -= 8;
			}

			//  Test for end of string
			if (bsBase.eos() && BufferedBits == 0) EndOfStream = true;

			//  Return the string
			return uint16_t(Acc);
		}

		//  next16
		//
		//  Writes the next bit string (up to 16 bits) to the stream
		//
		//  PARAMETERS
		//
		//		uint16_t			-   Value to be written
		//		size_t				-	Size of the bit string to read	
		// 
		//	RETURNS
		//
		//	NOTES
		//

		void next16(uint16_t Out, uint32_t Bits)
		{
			unsigned int			Acc = 0;									//  Bit string accumulator

			//  Load the existing BYTE array into the accumulator
			Acc = ByteArray[0];
			Acc <<= 8;
			Acc |= ByteArray[1];
			Acc <<= 8;
			Acc |= ByteArray[2];

			//  Accumulate the output bits
			Acc |= (Out << BitOffset);

			//  Store back the accumulated value
			ByteArray[2] = BYTE(Acc & 0x000000FF);
			ByteArray[1] = BYTE((Acc >> 8) & 0x000000FF);
			ByteArray[0] = BYTE((Acc >> 16) & 0x000000FF);

			BitOffset += Bits;

			//  Output any complete BYTES
			while (BitOffset >= 8)
			{
				bsBase.next(ByteArray[2]);
				ByteArray[2] = ByteArray[1];
				ByteArray[1] = ByteArray[0];
				ByteArray[0] = 0;
				BitOffset -= 8;
			}
			return;
		}
	};
}
