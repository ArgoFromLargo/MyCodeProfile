/**
 * Programming Languages - Spring 2017
 * @author Luke Kledzik
 *
 * This program reads in the nonogram's info from a textfile
 * that is specified as a command line argument. The program
 * generates a puzzle at the desired dimensions then proceeds
 * to solve it and print the solution.
 */

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
        var hints : Array[Array[Char]] = getHints(rows, columns, scanner)

        // println("Row hints: " + rowHints.mkString(", "))
        // println("Column hints: " + columnHints.mkString(", "))

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

    def getHints(rows : Int, columns : Int, scanner : Scanner) : Array[Array[Char]] = {
        var hints = Array.ofDim[Char](rows + columns, rows + columns)
        var line : String = ""
        var x : Int = 0
        var y : Int = 0

        for(x <- 0 until rows + columns) {
            line = scanner.nextLine
        }

        return hints
    }
}