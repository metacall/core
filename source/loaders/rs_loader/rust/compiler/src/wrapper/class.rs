use std::any::*;
use std::cell::Ref;
use std::cell::RefCell;
use std::cell::RefMut;
use std::collections::HashMap;
use std::ffi::CStr;
use std::fmt;
use std::sync::Arc;
type Result<T, E = i32> = core::result::Result<T, E>;
use std::os::raw::{c_char, c_double, c_float, c_int, c_long, c_short, c_void};
extern "C" {
    fn value_type_count(v: *mut c_void) -> c_int;
    fn value_type_id(v: *mut c_void) -> c_int;
    // fn metacall_value_id(v: *mut c_void) -> c_int;
    fn metacall_value_to_int(v: *mut c_void) -> c_int;
    // fn metacall_value_to_bool(v: *mut c_void) -> c_int;
    fn metacall_value_to_char(v: *mut c_void) -> c_char;
    fn metacall_value_to_long(v: *mut c_void) -> c_long;
    fn metacall_value_to_short(v: *mut c_void) -> c_short;
    fn metacall_value_to_float(v: *mut c_void) -> c_float;
    fn metacall_value_to_double(v: *mut c_void) -> c_double;
    fn metacall_value_to_array(v: *mut c_void) -> *mut *mut c_void;
    fn metacall_value_to_map(v: *mut c_void) -> *mut *mut c_void;
    // fn metacall_value_to_ptr(v: *mut c_void) -> *mut c_void;
    fn metacall_value_to_string(v: *mut c_void) -> *mut c_char;
    // fn metacall_function(cfn: *const c_char) -> *mut c_void;
    fn metacall_value_create_int(i: c_int) -> *mut c_void;
    fn metacall_value_create_bool(b: c_int) -> *mut c_void;
    fn metacall_value_create_long(l: c_long) -> *mut c_void;
    fn metacall_value_create_char(st: c_char) -> *mut c_void;
    fn metacall_value_create_short(s: c_short) -> *mut c_void;
    fn metacall_value_create_float(f: c_float) -> *mut c_void;
    fn metacall_value_create_double(d: c_double) -> *mut c_void;
    fn metacall_value_create_string(st: *const c_char, ln: usize) -> *mut c_void;
    fn metacall_value_create_array(values: *const *mut c_void, size: usize) -> *mut c_void;
    fn metacall_value_create_map(tuples: *const *mut c_void, size: usize) -> *mut c_void;
}

type Attributes = HashMap<&'static str, AttributeGetter>;
type AttributeSetters = HashMap<&'static str, AttributeSetter>;
type ClassMethods = HashMap<&'static str, ClassMethod>;
type InstanceMethods = HashMap<&'static str, InstanceMethod>;
pub type MetacallValue = *mut c_void;

#[derive(Clone)]
pub struct Class {
    /// The class name. Defaults to the `std::any::type_name`
    pub name: String,
    pub type_id: TypeId,
    constructor: Option<Constructor>,
    attributes: Attributes,
    attr_setters: AttributeSetters,
    instance_methods: InstanceMethods,
    pub class_methods: ClassMethods,
}

impl Class {
    pub fn builder<T: 'static>() -> ClassBuilder<T> {
        ClassBuilder::new()
    }

    pub fn init(&self, fields: Vec<MetacallValue>) -> Instance {
        self.constructor.as_ref().unwrap().invoke(fields).unwrap()
    }

    pub fn call(&self, attr: &str, args: Vec<MetacallValue>) -> Result<MetacallValue> {
        let attr = self.class_methods.get(attr).unwrap();

        attr.clone().invoke(args)
    }

    fn get_method(&self, name: &str) -> Option<InstanceMethod> {
        self.instance_methods.get(name).cloned()
    }
}

#[derive(Clone)]
pub struct ClassBuilder<T> {
    class: Class,
    /// A type marker. Used to ensure methods have the correct type.
    ty: std::marker::PhantomData<T>,
}
impl<T> ClassBuilder<T>
where
    T: 'static,
{
    /// Create a new class builder.
    fn new() -> Self {
        let fq_name = std::any::type_name::<T>().to_string();
        let short_name = fq_name.split("::").last().expect("type has invalid name");
        Self {
            class: Class {
                name: short_name.to_string(),
                constructor: None,
                attributes: Attributes::new(),
                attr_setters: AttributeSetters::new(),
                instance_methods: InstanceMethods::new(),
                class_methods: ClassMethods::new(),
                type_id: TypeId::of::<T>(),
            },
            ty: std::marker::PhantomData,
        }
    }
    /// Set the name of the polar class.
    pub fn name(mut self, name: &str) -> Self {
        self.class.name = name.to_string();
        self
    }

    /// Finish building a build the class
    pub fn build(self) -> Class {
        self.class
    }

    pub fn add_attribute_getter<F, R>(mut self, name: &'static str, f: F) -> Self
    where
        F: Fn(&T) -> R + Send + Sync + 'static,
        R: ToMetaResult,
        T: 'static,
    {
        self.class.attributes.insert(name, AttributeGetter::new(f));
        self
    }

    pub fn add_attribute_setter<F, Arg>(mut self, name: &'static str, f: F) -> Self
    where
        Arg: FromMeta,
        F: Fn(Arg, &mut T) + 'static,
        T: 'static,
    {
        self.class
            .attr_setters
            .insert(name, AttributeSetter::new(f));
        self
    }

    pub fn with_constructor<F, Args>(f: F) -> Self
    where
        F: Function<Args, Result = T>,
        T: Send + Sync,
        Args: FromMetaList,
    {
        let mut class: ClassBuilder<T> = ClassBuilder::new();
        class = class.set_constructor(f);
        class
    }

    pub fn set_constructor<F, Args>(mut self, f: F) -> Self
    where
        F: Function<Args, Result = T>,
        T: Send + Sync,
        Args: FromMetaList,
    {
        self.class.constructor = Some(Constructor::new(f));
        self
    }

    pub fn add_method<F, Args, R>(mut self, name: &'static str, f: F) -> Self
    where
        Args: FromMetaList,
        F: Method<T, Args, Result = R>,
        R: ToMetaResult + 'static,
    {
        self.class
            .instance_methods
            .insert(name, InstanceMethod::new(f));
        self
    }

    pub fn add_class_method<F, Args, R>(mut self, name: &'static str, f: F) -> Self
    where
        F: Function<Args, Result = R>,
        Args: FromMetaList + std::fmt::Debug,
        R: ToMetaResult + std::fmt::Debug + 'static,
    {
        self.class.class_methods.insert(name, ClassMethod::new(f));
        self
    }
}
#[derive(Clone)]
pub struct Instance {
    inner: Arc<RefCell<dyn std::any::Any + Send + Sync>>,
    debug_type_name: &'static str,
}

impl fmt::Debug for Instance {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Instance<{}>", self.debug_type_name)
    }
}

impl Instance {
    /// Create a new instance
    pub fn new<T: Send + Sync + 'static>(instance: T) -> Self {
        Self {
            inner: Arc::new(RefCell::new(instance)),
            debug_type_name: std::any::type_name::<T>(),
        }
    }

    /// Check whether this is an instance of `class`
    pub fn instance_of(&self, class: &Class) -> bool {
        self.type_id() == class.type_id
    }

    pub fn type_id(&self) -> std::any::TypeId {
        self.inner.as_ref().type_id()
    }

    /// Get the canonical name of this instance.
    pub fn name<'a>(&self) -> &'a str {
        self.debug_type_name
    }

    /// Lookup an attribute on the instance via the registered `Class`
    pub fn get_attr(&self, name: &str, class: &Class) -> Result<MetacallValue> {
        let attr = class.attributes.get(name).unwrap().clone();
        attr.invoke(self)
    }
    pub fn set_attr(&mut self, name: &str, value: MetacallValue, class: &Class) {
        let attr = class.attr_setters.get(name).unwrap().clone();
        attr.invoke(value, self)
    }

    pub fn borrow(&self) -> Ref<dyn std::any::Any + Send + Sync> {
        self.inner.as_ref().borrow()
    }

    pub fn borrow_mut(&self) -> RefMut<dyn std::any::Any + Send + Sync> {
        self.inner.as_ref().borrow_mut()
    }

    pub fn call(
        &self,
        name: &str,
        args: Vec<MetacallValue>,
        class: &Class,
    ) -> Result<MetacallValue> {
        let method = class.get_method(name).unwrap();
        method.invoke(self, args)
    }
}

pub trait Function<Args = ()>: Send + Sync + 'static {
    type Result;

    fn invoke(&self, args: Args) -> Self::Result;
}

/// Similar to a `Function` but also takes an explicit `receiver`
/// parameter than is the first argument of the call (i.e. the `self` param);
pub trait Method<Receiver, Args = ()>: Send + Sync + 'static {
    type Result;

    fn invoke(&self, receiver: &Receiver, args: Args) -> Self::Result;
}

macro_rules! tuple_impls {
    ( $( $name:ident )* ) => {
        impl<Fun, Res, $($name),*> Function<($($name,)*)> for Fun
        where
            Fun: Fn($($name),*) -> Res + Send + Sync + 'static
        {
            type Result = Res;

            fn invoke(&self, args: ($($name,)*)) -> Self::Result {
                #[allow(non_snake_case)]
                let ($($name,)*) = args;
                (self)($($name,)*)
            }
        }

        impl<Fun, Res, Receiver, $($name),*> Method<Receiver, ($($name,)*)> for Fun
        where
            Fun: Fn(&Receiver, $($name),*) -> Res + Send + Sync + 'static,
        {
            type Result = Res;

            fn invoke(&self, receiver: &Receiver, args: ($($name,)*)) -> Self::Result {
                #[allow(non_snake_case)]
                let ($($name,)*) = args;
                (self)(receiver, $($name,)*)
            }
        }
    };
}

tuple_impls! {}
tuple_impls! { A }
tuple_impls! { A B }
tuple_impls! { A B C }
tuple_impls! { A B C D }
tuple_impls! { A B C D E }
tuple_impls! { A B C D E F }
tuple_impls! { A B C D E F G }
tuple_impls! { A B C D E F G H }
tuple_impls! { A B C D E F G H I }
tuple_impls! { A B C D E F G H I J }
tuple_impls! { A B C D E F G H I J K }
tuple_impls! { A B C D E F G H I J K L }
tuple_impls! { A B C D E F G H I J K L M }
tuple_impls! { A B C D E F G H I J K L M N }
tuple_impls! { A B C D E F G H I J K L M N O }
tuple_impls! { A B C D E F G H I J K L M N O P }

fn join<A, B>(left: Result<A>, right: Result<B>) -> Result<(A, B)> {
    left.and_then(|l| right.map(|r| (l, r)))
}

type TypeErasedFunction<R> = Arc<dyn Fn(Vec<MetacallValue>) -> Result<R> + Send + Sync>;
type TypeErasedMethod<R> = Arc<dyn Fn(&Instance, Vec<MetacallValue>) -> Result<R> + Send + Sync>;

#[derive(Clone)]
pub struct Constructor(TypeErasedFunction<Instance>);

impl Constructor {
    pub fn new<Args, F>(f: F) -> Self
    where
        Args: FromMetaList,
        F: Function<Args>,
        F::Result: Send + Sync + 'static,
    {
        Constructor(Arc::new(move |args: Vec<MetacallValue>| {
            Args::from_meta_list(&args).map(|args| Instance::new(f.invoke(args)))
        }))
    }

    pub fn invoke(&self, args: Vec<MetacallValue>) -> Result<Instance> {
        self.0(args)
    }
}

#[derive(Clone)]
pub struct AttributeGetter(Arc<dyn Fn(&Instance) -> Result<MetacallValue> + Send + Sync>);
impl AttributeGetter {
    pub fn new<T, F, R>(f: F) -> Self
    where
        T: 'static,
        F: Fn(&T) -> R + Send + Sync + 'static,
        R: ToMetaResult,
    {
        Self(Arc::new(move |receiver| {
            let borrowed_receiver = receiver.borrow();
            let receiver = Ok(borrowed_receiver.downcast_ref::<T>().unwrap());
            receiver.map(&f).and_then(|v| v.to_meta_result())
        }))
    }

    pub fn invoke(&self, receiver: &Instance) -> Result<MetacallValue> {
        self.0(receiver)
    }
}

#[derive(Clone)]
pub struct AttributeSetter(Arc<dyn Fn(MetacallValue, &mut Instance)>);
impl AttributeSetter {
    pub fn new<T, F, Arg>(f: F) -> Self
    where
        T: 'static,
        Arg: FromMeta,
        F: Fn(Arg, &mut T) + 'static,
    {
        Self(Arc::new(move |value, receiver| {
            let mut borrowed_receiver = receiver.borrow_mut();
            let receiver = borrowed_receiver.downcast_mut::<T>().unwrap();
            f(FromMeta::from_meta(value).unwrap(), receiver)
        }))
    }

    pub fn invoke(&self, value: MetacallValue, receiver: &mut Instance) {
        self.0(value, receiver)
    }
}

#[derive(Clone)]
pub struct InstanceMethod(TypeErasedMethod<MetacallValue>);

impl InstanceMethod {
    pub fn new<T, F, Args>(f: F) -> Self
    where
        Args: FromMetaList,
        F: Method<T, Args>,
        F::Result: ToMetaResult,
        T: 'static,
    {
        Self(Arc::new(
            move |receiver: &Instance, args: Vec<MetacallValue>| {
                let borrowed_receiver = receiver.borrow();
                let receiver = Ok(borrowed_receiver.downcast_ref::<T>().unwrap());

                let args = Args::from_meta_list(&args);

                join(receiver, args)
                    .and_then(|(receiver, args)| f.invoke(receiver, args).to_meta_result())
            },
        ))
    }

    pub fn invoke(&self, receiver: &Instance, args: Vec<MetacallValue>) -> Result<MetacallValue> {
        self.0(receiver, args)
    }
}

#[derive(Clone)]
pub struct ClassMethod(TypeErasedFunction<MetacallValue>);

impl ClassMethod {
    pub fn new<F, Args>(f: F) -> Self
    where
        Args: FromMetaList + std::fmt::Debug,
        F: Function<Args>,
        F::Result: ToMetaResult + std::fmt::Debug,
    {
        Self(Arc::new(move |args: Vec<MetacallValue>| {
            Args::from_meta_list(&args).and_then(|args| {
                let res = f.invoke(args);
                res.to_meta_result()
            })
        }))
    }

    pub fn invoke(&self, args: Vec<MetacallValue>) -> Result<MetacallValue> {
        self.0(args)
    }
}

#[derive(Clone)]
pub struct NormalFunction(TypeErasedFunction<MetacallValue>);

impl NormalFunction {
    pub fn new<F, Args>(f: F) -> Self
    where
        Args: FromMetaList + std::fmt::Debug,
        F: Function<Args>,
        F::Result: ToMetaResult + std::fmt::Debug,
    {
        Self(Arc::new(move |args: Vec<MetacallValue>| {
            Args::from_meta_list(&args).and_then(|args| {
                let res = f.invoke(args);
                res.to_meta_result()
            })
        }))
    }

    pub fn invoke(&self, args: Vec<MetacallValue>) -> Result<MetacallValue> {
        self.0(args)
    }
}

pub trait ToMetaResult {
    fn to_meta_result(self) -> Result<MetacallValue>;
}

impl ToMetaResult for () {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_int(0) })
    }
}

// impl ToMetaResult for u32 {
//     fn to_meta_result(self) -> Result<MetacallValue> {
//         Ok(self as MetacallValue)
//     }
// }

impl ToMetaResult for bool {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_bool(self as i32) })
    }
}

impl ToMetaResult for char {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_char(self as i8) })
    }
}

impl ToMetaResult for usize {
    fn to_meta_result(self) -> Result<MetacallValue> {
        println!("get usize: {self}");
        // FIXME: convert usize to i32?
        Ok(unsafe { metacall_value_create_int(self as i32) })
    }
}

impl ToMetaResult for i8 {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_char(self) })
    }
}

impl ToMetaResult for i16 {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_short(self) })
    }
}

impl ToMetaResult for i32 {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_int(self) })
    }
}

impl ToMetaResult for i64 {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_long(self) })
    }
}

impl ToMetaResult for f32 {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_float(self) })
    }
}

impl ToMetaResult for f64 {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_double(self) })
    }
}

impl ToMetaResult for String {
    fn to_meta_result(self) -> Result<MetacallValue> {
        Ok(unsafe { metacall_value_create_string(self.as_ptr() as *const i8, self.len()) })
    }
}

impl<T> ToMetaResult for Vec<T>
where
    T: Clone + ToMetaResult,
{
    fn to_meta_result(self) -> Result<MetacallValue> {
        let ret_vec = self
            .into_iter()
            .map(|val| val.to_meta_result().unwrap())
            .collect::<Vec<*mut c_void>>();
        Ok(unsafe { metacall_value_create_array(ret_vec.as_ptr(), ret_vec.len()) })
    }
}

impl<K, V> ToMetaResult for HashMap<K, V>
where
    K: Clone + ToMetaResult,
    V: Clone + ToMetaResult,
{
    fn to_meta_result(self) -> Result<MetacallValue> {
        unsafe {
            let size = self.len();
            let ret_map = self
                .into_iter()
                .map(|(key, val)| {
                    let pair = vec![key.to_meta_result().unwrap(), val.to_meta_result().unwrap()];
                    metacall_value_create_array(pair.as_ptr(), pair.len())
                })
                .collect::<Vec<*mut c_void>>();
            Ok(metacall_value_create_map(ret_map.as_ptr(), size))
        }
    }
}
pub trait FromMetaList {
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self>
    where
        Self: Sized;
}
pub trait FromMeta: Clone {
    fn from_meta(val: MetacallValue) -> Result<Self>;
}

impl FromMeta for MetacallValue {
    fn from_meta(val: MetacallValue) -> Result<Self> {
        Ok(val)
    }
}
// these types are not compatible
// impl FromMeta for u32 {
//     fn from_meta(val: MetacallValue) -> Result<Self> {
//         Ok(val as u32)
//     }
// }
// impl FromMeta for bool {
//     fn from_meta(val: MetacallValue) -> Result<Self> {
//         Ok(unsafe { metacall_value_to_bool(val) as bool })
//     }
// }
// impl FromMeta for char {
//     fn from_meta(val: MetacallValue) -> Result<Self> {
//         Ok(unsafe { metacall_value_to_char(val) as char })
//     }
// }

enum PrimitiveMetacallProtocolTypes {
    Short = 2,
    Int = 3,
    Long = 4,
    Float = 5,
    Double = 6,
}

use std::convert::TryFrom;

impl TryFrom<i32> for PrimitiveMetacallProtocolTypes {
    type Error = ();

    fn try_from(v: i32) -> Result<Self, Self::Error> {
        match v {
            x if x == PrimitiveMetacallProtocolTypes::Short as i32 => Ok(PrimitiveMetacallProtocolTypes::Short),
            x if x == PrimitiveMetacallProtocolTypes::Int as i32 => Ok(PrimitiveMetacallProtocolTypes::Int),
            x if x == PrimitiveMetacallProtocolTypes::Long as i32 => Ok(PrimitiveMetacallProtocolTypes::Long),
            x if x == PrimitiveMetacallProtocolTypes::Float as i32 => Ok(PrimitiveMetacallProtocolTypes::Float),
            x if x == PrimitiveMetacallProtocolTypes::Double as i32 => Ok(PrimitiveMetacallProtocolTypes::Double),
            _ => Err(()),
        }
    }
}

macro_rules! convert_to {
    ($t:ty, $val:expr) => {
        unsafe {
            let id = value_type_id($val);

            match id.try_into() {
                Ok(PrimitiveMetacallProtocolTypes::Short) => Ok(metacall_value_to_short($val) as $t),
                Ok(PrimitiveMetacallProtocolTypes::Int) => Ok(metacall_value_to_int($val) as $t),
                Ok(PrimitiveMetacallProtocolTypes::Long) => Ok(metacall_value_to_long($val) as $t),
                Ok(PrimitiveMetacallProtocolTypes::Float) => Ok(metacall_value_to_float($val) as $t),
                Ok(PrimitiveMetacallProtocolTypes::Double) => Ok(metacall_value_to_double($val) as $t),
                Err(_) => {
                    println!("receive id: {}, should be [2-6]", id);
                    panic!("received mismatch type");
                }
            }
        }
    };
}

impl FromMeta for i8 {
    fn from_meta(val: MetacallValue) -> Result<Self> {
        Ok(unsafe { metacall_value_to_char(val) })
    }
}
impl FromMeta for i16 {
    fn from_meta(val: MetacallValue) -> Result<Self> {
        convert_to!(i16, val)
    }
}
impl FromMeta for i32 {
    fn from_meta(val: MetacallValue) -> Result<Self> {
        convert_to!(i32, val)
    }
}
impl FromMeta for i64 {
    fn from_meta(val: MetacallValue) -> Result<Self> {
        convert_to!(i64, val)
    }
}
impl FromMeta for f32 {
    fn from_meta(val: MetacallValue) -> Result<Self> {
        convert_to!(f32, val)
    }
}
impl FromMeta for f64 {
    fn from_meta(val: MetacallValue) -> Result<Self> {
        convert_to!(f64, val)
    }
}

impl FromMeta for String {
    fn from_meta(val: MetacallValue) -> Result<Self> {
        Ok(unsafe {
            let s = metacall_value_to_string(val);
            CStr::from_ptr(s).to_str().unwrap().to_owned()
        })
    }
}

impl<T> FromMeta for Vec<T>
where
    T: Clone + FromMeta,
{
    fn from_meta(val: MetacallValue) -> Result<Self> {
        Ok(unsafe {
            let arr = metacall_value_to_array(val);
            let count = value_type_count(val);
            let vec = std::slice::from_raw_parts(arr, count as usize)
                .iter()
                .map(|p| FromMeta::from_meta(*p).unwrap())
                .collect::<Vec<T>>()
                .clone();
            vec
        })
    }
}

impl<K, V> FromMeta for HashMap<K, V>
where
    K: Clone + FromMeta + std::cmp::Eq + std::hash::Hash,
    V: Clone + FromMeta,
{
    fn from_meta(val: MetacallValue) -> Result<Self> {
        Ok(unsafe {
            let map = metacall_value_to_map(val);
            let count = value_type_count(val);
            let map = std::slice::from_raw_parts(map, count as usize);
            let mut r_map: HashMap<K, V> = HashMap::new();
            for map_value in map {
                let m_pair = metacall_value_to_array(*map_value);
                let m_pair = std::slice::from_raw_parts(m_pair, 2);
                let key = FromMeta::from_meta(m_pair[0]).unwrap();
                let val = FromMeta::from_meta(m_pair[1]).unwrap();
                r_map.insert(key, val);
            }
            r_map
        })
    }
}

// impl FromMeta for &mut Vec<i32> {
//     fn from_meta(val: MetacallValue) -> Result<Self> {
//         Ok(unsafe {
//             let arr = metacall_value_to_array(val);
//             let count = value_type_count(val);
//             let vec = std::slice::from_raw_parts(arr, count as usize)
//                 .iter()
//                 .map(|p| metacall_value_to_int(*p))
//                 .collect::<Vec<i32>>();
//             println!("{:?}", vec);
//             vec
//         })
//     }
// }

#[allow(unused)]
impl FromMetaList for () {
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok(());
        result
    }
}
#[allow(unused)]
impl<TupleElement0: FromMeta> FromMetaList for (TupleElement0,) {
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((TupleElement0::from_meta(iter.next().unwrap().clone())?,));
        result
    }
}
#[allow(unused)]
impl<TupleElement0: FromMeta, TupleElement1: FromMeta> FromMetaList
    for (TupleElement0, TupleElement1)
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<TupleElement0: FromMeta, TupleElement1: FromMeta, TupleElement2: FromMeta> FromMetaList
    for (TupleElement0, TupleElement1, TupleElement2)
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
    > FromMetaList for (TupleElement0, TupleElement1, TupleElement2, TupleElement3)
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
        TupleElement7: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
        TupleElement7,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
            TupleElement7::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
        TupleElement7: FromMeta,
        TupleElement8: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
        TupleElement7,
        TupleElement8,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
            TupleElement7::from_meta(iter.next().unwrap().clone())?,
            TupleElement8::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
        TupleElement7: FromMeta,
        TupleElement8: FromMeta,
        TupleElement9: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
        TupleElement7,
        TupleElement8,
        TupleElement9,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
            TupleElement7::from_meta(iter.next().unwrap().clone())?,
            TupleElement8::from_meta(iter.next().unwrap().clone())?,
            TupleElement9::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
        TupleElement7: FromMeta,
        TupleElement8: FromMeta,
        TupleElement9: FromMeta,
        TupleElement10: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
        TupleElement7,
        TupleElement8,
        TupleElement9,
        TupleElement10,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
            TupleElement7::from_meta(iter.next().unwrap().clone())?,
            TupleElement8::from_meta(iter.next().unwrap().clone())?,
            TupleElement9::from_meta(iter.next().unwrap().clone())?,
            TupleElement10::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
        TupleElement7: FromMeta,
        TupleElement8: FromMeta,
        TupleElement9: FromMeta,
        TupleElement10: FromMeta,
        TupleElement11: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
        TupleElement7,
        TupleElement8,
        TupleElement9,
        TupleElement10,
        TupleElement11,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
            TupleElement7::from_meta(iter.next().unwrap().clone())?,
            TupleElement8::from_meta(iter.next().unwrap().clone())?,
            TupleElement9::from_meta(iter.next().unwrap().clone())?,
            TupleElement10::from_meta(iter.next().unwrap().clone())?,
            TupleElement11::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
        TupleElement7: FromMeta,
        TupleElement8: FromMeta,
        TupleElement9: FromMeta,
        TupleElement10: FromMeta,
        TupleElement11: FromMeta,
        TupleElement12: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
        TupleElement7,
        TupleElement8,
        TupleElement9,
        TupleElement10,
        TupleElement11,
        TupleElement12,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
            TupleElement7::from_meta(iter.next().unwrap().clone())?,
            TupleElement8::from_meta(iter.next().unwrap().clone())?,
            TupleElement9::from_meta(iter.next().unwrap().clone())?,
            TupleElement10::from_meta(iter.next().unwrap().clone())?,
            TupleElement11::from_meta(iter.next().unwrap().clone())?,
            TupleElement12::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
        TupleElement7: FromMeta,
        TupleElement8: FromMeta,
        TupleElement9: FromMeta,
        TupleElement10: FromMeta,
        TupleElement11: FromMeta,
        TupleElement12: FromMeta,
        TupleElement13: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
        TupleElement7,
        TupleElement8,
        TupleElement9,
        TupleElement10,
        TupleElement11,
        TupleElement12,
        TupleElement13,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
            TupleElement7::from_meta(iter.next().unwrap().clone())?,
            TupleElement8::from_meta(iter.next().unwrap().clone())?,
            TupleElement9::from_meta(iter.next().unwrap().clone())?,
            TupleElement10::from_meta(iter.next().unwrap().clone())?,
            TupleElement11::from_meta(iter.next().unwrap().clone())?,
            TupleElement12::from_meta(iter.next().unwrap().clone())?,
            TupleElement13::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
        TupleElement7: FromMeta,
        TupleElement8: FromMeta,
        TupleElement9: FromMeta,
        TupleElement10: FromMeta,
        TupleElement11: FromMeta,
        TupleElement12: FromMeta,
        TupleElement13: FromMeta,
        TupleElement14: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
        TupleElement7,
        TupleElement8,
        TupleElement9,
        TupleElement10,
        TupleElement11,
        TupleElement12,
        TupleElement13,
        TupleElement14,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
            TupleElement7::from_meta(iter.next().unwrap().clone())?,
            TupleElement8::from_meta(iter.next().unwrap().clone())?,
            TupleElement9::from_meta(iter.next().unwrap().clone())?,
            TupleElement10::from_meta(iter.next().unwrap().clone())?,
            TupleElement11::from_meta(iter.next().unwrap().clone())?,
            TupleElement12::from_meta(iter.next().unwrap().clone())?,
            TupleElement13::from_meta(iter.next().unwrap().clone())?,
            TupleElement14::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}
#[allow(unused)]
impl<
        TupleElement0: FromMeta,
        TupleElement1: FromMeta,
        TupleElement2: FromMeta,
        TupleElement3: FromMeta,
        TupleElement4: FromMeta,
        TupleElement5: FromMeta,
        TupleElement6: FromMeta,
        TupleElement7: FromMeta,
        TupleElement8: FromMeta,
        TupleElement9: FromMeta,
        TupleElement10: FromMeta,
        TupleElement11: FromMeta,
        TupleElement12: FromMeta,
        TupleElement13: FromMeta,
        TupleElement14: FromMeta,
        TupleElement15: FromMeta,
    > FromMetaList
    for (
        TupleElement0,
        TupleElement1,
        TupleElement2,
        TupleElement3,
        TupleElement4,
        TupleElement5,
        TupleElement6,
        TupleElement7,
        TupleElement8,
        TupleElement9,
        TupleElement10,
        TupleElement11,
        TupleElement12,
        TupleElement13,
        TupleElement14,
        TupleElement15,
    )
{
    fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
        let mut iter = values.iter();
        let result = Ok((
            TupleElement0::from_meta(iter.next().unwrap().clone())?,
            TupleElement1::from_meta(iter.next().unwrap().clone())?,
            TupleElement2::from_meta(iter.next().unwrap().clone())?,
            TupleElement3::from_meta(iter.next().unwrap().clone())?,
            TupleElement4::from_meta(iter.next().unwrap().clone())?,
            TupleElement5::from_meta(iter.next().unwrap().clone())?,
            TupleElement6::from_meta(iter.next().unwrap().clone())?,
            TupleElement7::from_meta(iter.next().unwrap().clone())?,
            TupleElement8::from_meta(iter.next().unwrap().clone())?,
            TupleElement9::from_meta(iter.next().unwrap().clone())?,
            TupleElement10::from_meta(iter.next().unwrap().clone())?,
            TupleElement11::from_meta(iter.next().unwrap().clone())?,
            TupleElement12::from_meta(iter.next().unwrap().clone())?,
            TupleElement13::from_meta(iter.next().unwrap().clone())?,
            TupleElement14::from_meta(iter.next().unwrap().clone())?,
            TupleElement15::from_meta(iter.next().unwrap().clone())?,
        ));
        result
    }
}

// #[impl_for_tuples(16)]
// #[tuple_types_custom_trait_bound(FromMeta)]
// impl FromMetaList for Tuple {
//     fn from_meta_list(values: &[MetacallValue]) -> Result<Self> {
//         let mut iter = values.iter();
//         let result = Ok((for_tuples!(
//             #( Tuple::from_meta(iter.next().unwrap().clone())? ),*
//         )));
//         result
//     }
// }
