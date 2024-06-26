# Undefined Behavior

### Example 1 - Writing to not allocated memmory
[GODBOLT](https://godbolt.org/z/c4sseTG8n)

* CPP
    - It would lead to undefined behavior because the algorithm attempts to write to memory locations that have not been allocated or are not owned by the dst vector.
```cpp
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

auto f(const std::vector<int>& src) -> std::vector<int> {
    std::vector<int> dst;
    //%dst.reserve(src.size()); // Reserve space to avoid reallocations
    //% std::transform(src.begin(), src.end(), std::back_inserter(dst), [](int i) {
    //%     return std::pow(i, 2);
    //% });
    std::transform(src.begin(), src.end(), dst.begin(), [](int i) {
        return std::pow(i, 2);
    });
    return dst;
}

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto res = f(vec);
    for (const auto& v : res) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    return 0;
}
```

* RUST
    - Rust's compiler and type system prevent the kind of mistake seen in the C++ example, but for educational purposes, let's start with an attempt that might resemble the misuse of std::transform with an uninitialized destination:
    - Rust does not allow uninitialized memory access or buffer overflow by design. The language's safety guarantees and compile-time checks ensure that operations on collections like vectors are performed within the bounds of allocated memory, preventing undefined behavior related to memory access.
```rust,editable
fn f(src: &Vec<i32>) -> Vec<i32> {
    let mut dst: Vec<i32>;
    #//let mut dst: Vec<i32> = Vec::with_capacity(src.len());
    src.iter()
        .map(|&x| x.pow(2))
        .for_each(|xx| dst.push(xx));
    dst
}

pub fn main() {
    let src = vec![1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    let res = f(&src);
    println!("{:?}", res);
}
```

To prevent the problem in C++, according to the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-init), one should not declare a variable before initializing it.
The idiomatic way to write this code in C++23 would be

```cpp
#include <iostream>
#include <ranges>
#include <vector>
#include <cmath>

using namespace std;

auto f(const std::vector<int>& src) -> std::vector<int> {
    return src | views::transform([](int v) { return pow(v, 2); }) | ranges::to<std::vector<int>>();
}

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto res = f(vec);
    for (const auto& v : res) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    return 0;
}

```

It's also nicer to write this way in Rust, becuase it doesn't require dst to be mutable, and is shorter and more succint

```rust,editable
fn f(src: &Vec<i32>) -> Vec<i32> {
    src.iter().map(|&x| x.pow(2)).collect()
}

pub fn main() {
    let src = vec![1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    let res = f(&src);
    println!("{:?}", res);
}

```


### Example 2 - Object Slicing and Polymorphism
[GODBOLT](https://godbolt.org/z/E4jefbYYv)
* CPP
Object slicing occurs when an object of a derived class is assigned to a base class object, leading to the loss of the derived part of the object. This can be particularly insidious in C++ because it often doesn't prevent compilation, but it can lead to unexpected runtime behavior.
```cpp
#include <iostream>
#include <memory>
#include <vector>

class Base {
public:
    virtual void print() const {
        std::cout << "Base class" << std::endl;
    }
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    void print() const override {
        std::cout << "Derived class" << std::endl;
    }
};

void process(const Base& obj) {
    obj.print(); // Polymorphic call
}

int main() {
    Derived d;
    process(d); // Expected polymorphic behavior

    std::vector<Base> vec;
    vec.push_back(d); // Object slicing here
    vec.back().print(); // Calls Base::print() instead of Derived::print(), due to object slicing

    //% std::vector<std::unique_ptr<Base>> vec;
    //% vec.push_back(std::make_unique<Derived>(d)); // No object slicing, polymorphism preserved
    //% vec.back()->print(); // Calls Derived::print(), preserving polymorphism

    return 0;
}
```

* RUST
In Rust, preventing object slicing and ensuring polymorphism works as expected can be achieved by using trait objects or generics, along with smart pointers like Box, which enable dynamic dispatch
```rust,editable
use std::fmt::Debug;

trait Printable: Debug {
    fn print(&self);
}

#[derive(Debug)]
struct Base;
impl Printable for Base {
    fn print(&self) {
        println!("Base struct");
    }
}

#[derive(Debug)]
struct Derived;
impl Printable for Derived {
    fn print(&self) {
        println!("Derived struct");
    }
}

fn process(obj: &dyn Printable) {
    obj.print(); // Polymorphic call
}

pub fn main() {
    let d = Derived;
    process(&d); // Polymorphism in action

    let mut vec: Vec<Box<dyn Printable>> = Vec::new();
    vec.push(Box::new(d)); // No object slicing, dynamic dispatch works

    vec[0].print(); // Calls Derived::print(), preserving polymorphism

    // Using the debug trait to illustrate that the whole object is preserved
    println!("{:?}", vec[0]);
}
```



### Example 3 - Lambdas - Dangling Reference
[GODBOLT](https://godbolt.org/z/fqWr4coff)
* CPP
    - Capturing local variables by reference in a lambda that outlives the scope of those variables typically leads to a dangling reference. Accessing a dangling reference is undefined behavior because the variable it refers to no longer exists.
```cpp
#include <iostream>

auto f() {
    int v = 42;
    return [&]() {
    //%//return [=]() mutable {
        v += 100;
        return v;
    };
}

int main() {
    auto res = f();
    std::cout << res() << std::endl;
    return 0;
}
```

* RUST
    - Rust's design inherently prevents the creation of dangling references or captures by ensuring that any captured variables live as long as the closure itself. This is achieved through Rust's ownership and borrowing rules, which enforce compile-time checks for lifetimes and ownership.
```rust,editable
fn f() -> impl Fn() -> i32 {
    let v = 42;
    || {
        // This will not compile because `v` does not live long enough
        v + 100
    }
}

#// Rust forces us to use Captured Variables by Value
#// fn f() -> impl Fn() -> i32 {
#//     let v = 42;
#//     {
#//         v + 100
#//     }
#// }

pub fn main() {
    let res = f();
    println!("{}", res());
}
```



### Example 4 - Dangling iterators
[GODBOLT](https://godbolt.org/z/c7rsTj7cP)
* CPP
    - Erasing elements from a container (e.g., using erase method) invalidates iterators pointing to the erased elements and potentially beyond, depending on the container type.
```cpp
#include <iostream>
#include <vector>
#include <string>

int main() {
    std::vector<int> v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto it_beg = v.begin();
    auto it = v.begin() + 4;
    auto it_last = v.end();
    v.erase(it);                  // 'it' is invalidated
    std::cout << "1) it_beg: "<< *it_beg << " it: " << *it << " it_last: " << *it_last << std::endl;  // Accessing 'it_s' now leads to undefined behavior
    v.erase(it_beg);
    std::cout << "2) it_beg: "<< *it_beg << " it: " << *it << " it_last: " << *it_last << std::endl;  // Accessing 'it_s' now leads to undefined behavior
    v.erase(it);
    std::cout << "3) it_beg: "<< *it_beg << " it: " << *it << " it_last: " << *it_last << std::endl;  // Accessing 'it_s' now leads to undefined behavior
    // v.erase(it);
    // std::cout << "4) it_beg: "<< *it_beg << " it: " << *it << " it_last: " << *it_last << std::endl;  // Accessing 'it_s' now leads to undefined behavior
    // v.erase(it);
    // std::cout << "5) it_beg: "<< *it_beg << " it: " << *it << " it_last: " << *it_last << std::endl;  // Accessing 'it_s' now leads to undefined behavior
    // v.erase(it);
    // std::cout << "6) it_beg: "<< *it_beg << " it: " << *it << " it_last: " << *it_last << std::endl;  // Accessing 'it_s' now leads to undefined behavior
    // v.erase(it);
    // std::cout << "7) it_beg: "<< *it_beg << " it: " << *it << " it_last: " << *it_last << std::endl;  // Accessing 'it_s' now leads to undefined behavior
    return 0;
}
```

* RUST
```rust,editable
pub fn main() {
    let mut v = vec![0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

    v.remove(4); // This is analogous to v.erase(it) in C++

    // Safe accesses using `get`
    println!("1) it_beg: {:?}, it: {:?}, it_last: {:?}", v.get(0), v.get(4), v.get(v.len()));

    v.remove(0); // Removes the first element, shifting all others left
    println!("2) it_beg: {:?}, it: {:?}, it_last: {:?}", v.get(0), v.get(4), v.get(v.len()));

    // Attempt to remove an element at a now-invalid index (handled safely)
    // This line would panic if we directly indexed, but with `get` we can see it returns `None`
    match v.get(4) {
        Some(&element) => {
            v.remove(4); // Safe if element exists
            println!("Element at index 4 removed");
        },
        None => println!("No element at index 4, cannot remove"),
    }

    println!("3) it_beg: {:?}, it: {:?}, it_last: {:?}", v.get(0), v.get(4), v.get(v.len()));

    #//v.remove(9); // Removes the first element, shifting all others left
}
```


### Example 5 - Maybe not undefined but weird std::map operator [] behavior
[GODBOLT](https://godbolt.org/z/vnajbza4z)
* CPP
    - When you use the indexing operator ([]) on a std::map in C++ to access an element by its key, and if that key does not exist in the map, a new element with that key will be automatically created and initialized to its default value.
```cpp
#include <iostream>
#include <map>
#include <string>

//%// using EngineConfigMap = std::map<std::string, int>;

//%// class EngineController {

//%// public:
//%//     EngineController(const EngineConfigMap& config)
//%//     : m_config(config)
//%//     {
//%//         std::cout << "Initializing with default config: \n"
//%//                 << "max_temp: " << m_config["timeout"] << std::endl
//%//                 << "max_rpm: " << m_config["max_rpm"] << std::endl;
//%//     }
//%// private:
//%//     EngineConfigMap m_config;
//%// };

int main() {
    std::map<std::string, int> ids_map;
    ids_map["id1"] = 12;
    std::cout << ids_map["id2"] << std::endl;

    //%// EngineConfigMap empty_config;
    //%// EngineController ec(empty_config);

    return 0;
}
```

* RUST
    - If "id2" does not exist in ids_map, it will be inserted with a default value of 0, and then the value is printed. This approach is idiomatic in Rust and provides a safe and explicit way to handle potential missing keys in HashMaps.
```rust,editable
use std::collections::HashMap;

pub fn main() {
    let mut ids_map = HashMap::new();
    ids_map.insert("id1".to_string(), 12);

    // Using entry() and or_insert() to insert a default value for "id2" if it doesn't exist
    let id2_value = ids_map.entry("id2".to_string()).or_insert(0);
    println!("{}", id2_value);
}
```
