{-# LANGUAGE ForeignFunctionInterface #-}

module MetacallHSBridge where

import Data.IORef (IORef, modifyIORef', newIORef, readIORef, writeIORef)
import Data.Word (Word8)
import Foreign (Ptr, Storable(..), castPtr, nullPtr, plusPtr)
import Foreign.C (CInt(..), CSize(..), CString, newCString)
import Foreign.Marshal.Alloc (free)
import Foreign.Marshal.Utils (fillBytes)
import System.IO.Unsafe (unsafePerformIO)

typeIdBool, typeIdChar, typeIdShort, typeIdInt, typeIdLong, typeIdFloat, typeIdDouble, typeIdString, typeIdBuffer, typeIdPtr, typeIdNull :: CInt
typeIdBool = 0
typeIdChar = 1
typeIdShort = 2
typeIdInt = 3
typeIdLong = 4
typeIdFloat = 5
typeIdDouble = 6
typeIdString = 7
typeIdBuffer = 8
typeIdPtr = 11
typeIdNull = 14

typeIdInvalid :: CInt
typeIdInvalid = 20

data HSLoaderFFIValue

data FFIBytes = FFIBytes (Ptr ()) CSize

instance Storable FFIBytes where
	sizeOf _ = alignUp (sizeOf (undefined :: Ptr ())) (alignment (undefined :: CSize)) + sizeOf (undefined :: CSize)
	alignment _ = max (alignment (undefined :: Ptr ())) (alignment (undefined :: CSize))
	peek ptr = do
		p <- peekByteOff ptr 0
		s <- peekByteOff ptr (alignUp (sizeOf (undefined :: Ptr ())) (alignment (undefined :: CSize)))
		return (FFIBytes p s)
	poke ptr (FFIBytes p s) = do
		pokeByteOff ptr 0 p
		pokeByteOff ptr (alignUp (sizeOf (undefined :: Ptr ())) (alignment (undefined :: CSize))) s

data ModuleState = ModuleState
	{ fnAnswer :: CString
	, fnAdd :: CString
	, argLeft :: CString
	, argRight :: CString
	, errNone :: CString
	, errBadFunction :: CString
	, errBadArguments :: CString
	, currentError :: CString
	}

{-# NOINLINE moduleStateRef #-}
moduleStateRef :: IORef (Maybe ModuleState)
moduleStateRef = unsafePerformIO (newIORef Nothing)

alignUp :: Int -> Int -> Int
alignUp x a = ((x + a - 1) `div` a) * a

ffiUnionAlign :: Int
ffiUnionAlign = maximum
	[ alignment (undefined :: Word8)
	, alignment (undefined :: CInt)
	, alignment (undefined :: CSize)
	, alignment (undefined :: Ptr ())
	, alignment (undefined :: Double)
	, alignment (undefined :: FFIBytes)
	]

ffiUnionSize :: Int
ffiUnionSize = maximum
	[ sizeOf (undefined :: Word8)
	, sizeOf (undefined :: CInt)
	, sizeOf (undefined :: CSize)
	, sizeOf (undefined :: Ptr ())
	, sizeOf (undefined :: Double)
	, sizeOf (undefined :: FFIBytes)
	]

ffiValueTypeOffset :: Int
ffiValueTypeOffset = 0

ffiValueUnionOffset :: Int
ffiValueUnionOffset = alignUp (sizeOf (undefined :: CInt)) ffiUnionAlign

ffiValueSize :: Int
ffiValueSize = ffiValueUnionOffset + ffiUnionSize

ffiValueTypeAt :: Ptr HSLoaderFFIValue -> IO CInt
ffiValueTypeAt ptr = peekByteOff (castPtr ptr :: Ptr Word8) ffiValueTypeOffset

ffiValueIntAt :: Ptr HSLoaderFFIValue -> IO CInt
ffiValueIntAt ptr = peekByteOff (castPtr ptr :: Ptr Word8) ffiValueUnionOffset

setFFIValueInt :: Ptr HSLoaderFFIValue -> CInt -> IO ()
setFFIValueInt ptr n = do
	let raw = castPtr ptr :: Ptr Word8
	fillBytes raw 0 ffiValueSize
	pokeByteOff raw ffiValueTypeOffset typeIdInt
	pokeByteOff raw ffiValueUnionOffset n

setCurrentError :: CString -> IO ()
setCurrentError errPtr =
	modifyIORef' moduleStateRef $ \state ->
		case state of
			Nothing -> Nothing
			Just s -> Just s { currentError = errPtr }

withState :: (ModuleState -> IO a) -> IO a
withState f = do
	state <- readIORef moduleStateRef
	case state of
		Nothing -> do
			_ <- metacall_hs_module_initialize
			state2 <- readIORef moduleStateRef
			case state2 of
				Nothing -> error "Haskell bridge state initialization failed"
				Just s -> f s
		Just s -> f s

metacall_hs_module_initialize :: IO CInt
metacall_hs_module_initialize = do
	state <- readIORef moduleStateRef
	case state of
		Just _ -> return 0
		Nothing -> do
			nameAnswer <- newCString "hs_answer"
			nameAdd <- newCString "hs_add"
			nameLeft <- newCString "left"
			nameRight <- newCString "right"
			errNonePtr <- newCString ""
			errBadFunctionPtr <- newCString "invalid function index"
			errBadArgumentsPtr <- newCString "invalid argument list"
			writeIORef moduleStateRef (Just ModuleState
				{ fnAnswer = nameAnswer
				, fnAdd = nameAdd
				, argLeft = nameLeft
				, argRight = nameRight
				, errNone = errNonePtr
				, errBadFunction = errBadFunctionPtr
				, errBadArguments = errBadArgumentsPtr
				, currentError = errNonePtr
				})
			return 0

metacall_hs_module_destroy :: IO ()
metacall_hs_module_destroy = do
	state <- readIORef moduleStateRef
	case state of
		Nothing -> return ()
		Just s -> do
			free (fnAnswer s)
			free (fnAdd s)
			free (argLeft s)
			free (argRight s)
			free (errNone s)
			free (errBadFunction s)
			free (errBadArguments s)
			writeIORef moduleStateRef Nothing

metacall_hs_module_function_count :: IO CSize
metacall_hs_module_function_count = return 2

metacall_hs_module_function_name :: CSize -> IO CString
metacall_hs_module_function_name idx = withState $ \s ->
	case idx of
		0 -> return (fnAnswer s)
		1 -> return (fnAdd s)
		_ -> return nullPtr

metacall_hs_module_function_args_count :: CSize -> IO CSize
metacall_hs_module_function_args_count idx =
	case idx of
		0 -> return 0
		1 -> return 2
		_ -> return 0

metacall_hs_module_function_arg_name :: CSize -> CSize -> IO CString
metacall_hs_module_function_arg_name fnIdx argIdx = withState $ \s ->
	case (fnIdx, argIdx) of
		(1, 0) -> return (argLeft s)
		(1, 1) -> return (argRight s)
		_ -> return nullPtr

metacall_hs_module_function_arg_type :: CSize -> CSize -> IO CInt
metacall_hs_module_function_arg_type fnIdx argIdx =
	case (fnIdx, argIdx) of
		(1, 0) -> return typeIdInt
		(1, 1) -> return typeIdInt
		_ -> return typeIdInvalid

metacall_hs_module_function_return_type :: CSize -> IO CInt
metacall_hs_module_function_return_type fnIdx =
	case fnIdx of
		0 -> return typeIdInt
		1 -> return typeIdInt
		_ -> return typeIdInvalid

metacall_hs_module_invoke :: CSize -> Ptr HSLoaderFFIValue -> CSize -> Ptr HSLoaderFFIValue -> IO CInt
metacall_hs_module_invoke fnIdx args argsCount ret = withState $ \s -> do
	if ret == nullPtr then do
		setCurrentError (errBadArguments s)
		return 1
	else
		case fnIdx of
			0 -> do
				setFFIValueInt ret 42
				setCurrentError (errNone s)
				return 0
			1 -> do
				if argsCount < 2 || args == nullPtr then do
					setCurrentError (errBadArguments s)
					return 1
				else do
					let firstArgPtr = args
					let secondArgPtr = firstArgPtr `plusPtr` ffiValueSize
					firstType <- ffiValueTypeAt firstArgPtr
					secondType <- ffiValueTypeAt secondArgPtr
					if firstType /= typeIdInt || secondType /= typeIdInt then do
						setCurrentError (errBadArguments s)
						return 1
					else do
						left <- ffiValueIntAt firstArgPtr
						right <- ffiValueIntAt secondArgPtr
						setFFIValueInt ret (left + right)
						setCurrentError (errNone s)
						return 0
			_ -> do
				setCurrentError (errBadFunction s)
				return 1

metacall_hs_module_value_destroy :: Ptr HSLoaderFFIValue -> IO ()
metacall_hs_module_value_destroy _ = return ()

metacall_hs_module_last_error :: IO CString
metacall_hs_module_last_error = do
	state <- readIORef moduleStateRef
	case state of
		Nothing -> return nullPtr
		Just s -> return (currentError s)

foreign export ccall metacall_hs_module_initialize :: IO CInt
foreign export ccall metacall_hs_module_destroy :: IO ()
foreign export ccall metacall_hs_module_function_count :: IO CSize
foreign export ccall metacall_hs_module_function_name :: CSize -> IO CString
foreign export ccall metacall_hs_module_function_args_count :: CSize -> IO CSize
foreign export ccall metacall_hs_module_function_arg_name :: CSize -> CSize -> IO CString
foreign export ccall metacall_hs_module_function_arg_type :: CSize -> CSize -> IO CInt
foreign export ccall metacall_hs_module_function_return_type :: CSize -> IO CInt
foreign export ccall metacall_hs_module_invoke :: CSize -> Ptr HSLoaderFFIValue -> CSize -> Ptr HSLoaderFFIValue -> IO CInt
foreign export ccall metacall_hs_module_value_destroy :: Ptr HSLoaderFFIValue -> IO ()
foreign export ccall metacall_hs_module_last_error :: IO CString
