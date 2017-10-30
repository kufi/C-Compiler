val CONSTANT: Int = 10;

var mutable: Int = CONSTANT;

def main(): Unit
{
  test();
  printPlus10(CONSTANT);
  printPlus10(double(15));
  val result: Int = compare(10, double(10));

  if(result > 10) {
    println(result);
    if(result > 20) {
      println("Result is even above 20");
    }
  } else {
    println("Result is below 10");
    if(result < 0) {
      println("Result is even below 0");
    }
  }

  var counter: Int = result;

  while(counter > threshold()) {
    counter = counter - 1;
    println("Counter is now " + counter);
  }

  println(result);
  println(mutable);
  mutable = mutable + 5;
  println(mutable + result);
}

def threshold(): Int
{
  return 0;
}

def test(): Unit
{
  val test: Int = 10;
  val test2: Int = test;
  var abc: Int;
  abc = test2 + test;
  println(abc);
}

def compare(first, second): Boolean
{
  return first < second;
}

def double(input): Int
{
  return input * 2;
}

def printPlus10(single): Unit
{
  val test: Int = 10 + single;
  println(test);
}
