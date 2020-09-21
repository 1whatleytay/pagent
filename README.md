# pagent - The UI Design Language

Pagent is a concept language for creating reactive webpages.
It has been put on the backburner due to the release of [hermes](https://github.com/1whatleytay/hermes) and a change of interests.
A demo is (or was) available on https://desgroup.me/

Projects involving UI tend to be split into many different parts of languages.
A webpage involves using at minimum HTML, CSS and Javascript, along with frameworks and server languages.
Pagent's goal is to simplify all these different components into one developer frontend.
Styling should be done by passing arguments, server endpoints should be methods, and HTML structure should be done in arrays of children.
I believe the project accomplished a good part of its goal.

## Syntax and Features
There are a lot of parts to Pagent, so I've wrote a sample program to demonstrate a few important parts.
```
// Declare a variable
var z = 0

// Declare a Function
fun myFunc() {
  // Write Statements
  z += 1

  // Declare locals
  var y = z * z

  // Write if statements
  if y > z + 10 {
    // Call functions
    changeInstance(instance)
  }
}

// Declare types
type MyType {
  // String Literals
  var helloWorld = "Hello World -> \(z)"

  // Define Methods
  fun z() {
    // Printing
    print(helloWorld)
  }

  // Constructors, optional types
  init(text: String?) {
    // `has` keyword to unpack optionals
    if has text {
      helloWorld = text
    }
  }
}

// Instantiate Types
var instance = MyType()

// Take parameters
fun changeInstance(x: MyType) {
  // Modify fields and call methods
  x.helloWorld = "4"
  x.z()
}

// Declare Webpages
page MainPage {
  // Set their routes
  route '/'

  // Define reactive variables
  var x = 0

  // Arrays and specific types
  var arr: [MyType] = []
  
  // Optional Semicolons and Commas in lists
  View {
    Text("Hello World \(x)!")

    // Buttons and Lambdas
    Button("Press Me", click: {
      x += 1

      arr.push(MyType())
    })

    // For Loops in place of `map` call
    for a in arr ->
      HelloComponent(inst: a, event: { arr.remove(a) }) // Named parameters, overloaded functions
  }
}

// Subcomponent, doesn't have a route, can be used by other components
page HelloComponent {
  // `init var` declares constructors that allow passing of a variable.
  // Convenience for vue props in some ways.
  init var inst: MyType

  // Function type, replaces vue events.
  // Can specify return type and parameters, Function(x: Number, y: MyType): Float
  init var event: Function

  // Expression will declare the content of this page.
  Clickable(click: event) {
    // Styling in parameters, scene for building horizontally, view for building vertically
    Scene(padding: Padding(x: 4, y: 2), color: Color.blue, rounded: 20) {
      Image("/icon/guineapig.png")
    }
  }
}
```