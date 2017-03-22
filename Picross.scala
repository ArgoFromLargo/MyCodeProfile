import java.util.Scanner
import java.io.File

object Picross{
    def main(args : Array[String]) = {
        var rows : Int = 0
        var columns : Int = 0

        confirmArgs(args)

        var scanner : Scanner = new Scanner(new File(args(0)))

        // Retreive number of rows and columns
        rows = scanner.nextInt
        columns = scanner.nextInt
        // Move scanner to next line to begin reading row/column info
        scanner.nextLine

        println(rows + " rows, " + columns + " columns.")

        var puzzle : Array[Array[Char]] = createPuzzle(rows, columns)
        var rowHints : Array[String] = getHints(scanner, rows)
        var columnHints : Array[String] = getHints(scanner, columns)

        println(rowHints.mkString(", "))
        println(columnHints.mkString(", "))

        printPuzzle(rows, columns, puzzle)

        scanner.close
    }

    def confirmArgs(args : Array[String]) : Unit = {
        if(args.length == 1) {
            println("Program start...")
        }
        else {
            println("Incorrect number of arguments.")
            println("Shutting down.")
            sys.exit(1)
        }
    }

    def createPuzzle(rows : Int, columns : Int) : Array[Array[Char]] = {
        var puzzle = Array.ofDim[Char](rows, columns)

        var x : Int = 0
        var y : Int = 0

        // Traverse through the 2D array
        for(x <- 0 until rows; y <- 0 until columns) {
            puzzle(x)(y) = '.'
        }

        return puzzle
    }

    def printPuzzle(rows : Int, columns : Int, puzzle : Array[Array[Char]]) : Unit = {
        var x : Int = 0
        var y : Int = 0

        // Traverse through the 2D array
        for(x <- 0 until rows; y <- 0 until columns) {
            print(puzzle(x)(y))

            if(y + 1 == columns) {
                println()
            }
        }
    }

    def getHints(scanner : Scanner, num : Int) : Array[String] = {
        var hints = new Array[String](num)
        var x : Int = 0

        for(x <- 0 until num) {
            hints(x) = scanner.nextLine
        }

        return hints
    }
}
