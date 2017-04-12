import java.util.Scanner
import java.io._

object Picross{
    def main(args : Array[String]) = {
        var rows : Int = 0
        var columns : Int = 0

        confirmArgs(args)

        println("Program start...")

        var scanner : Scanner = new Scanner(new File(args(0)))

        // Retrieve number of rows and columns
        rows = scanner.nextInt
        columns = scanner.nextInt
        // Move scanner to next line to begin reading row/column info
        scanner.nextLine

        println(rows + " rows, " + columns + " columns.")

        // var puzzle : Array[Array[Char]] = createPuzzle(rows, columns)
        // var hints : Array[Array[Int]] = getHints(rows, columns, scanner)

        // printPuzzle(rows, columns, puzzle)

        // println("Row hints: " + rowHints.mkString(", "))
        // println("Column hints: " + columnHints.mkString(", "))

        // printHints(rows, columns, hints)

        // var numHints : Array[Int] = getNumHints(rows + columns, hints)

        solvePuzzle(rows, columns)

        scanner.close
    }

    def confirmArgs(args : Array[String]) : Unit = {
        if(args.length != 1) {
            println("Incorrect number of arguments.")
            println("Shutting down.")
            sys.exit(1)
        }
    }

    def createPuzzle(rows : Int, columns : Int) : Array[Array[Char]] = {
        Array.fill(rows, columns){'.'}
    }

    def printPuzzle(rows : Int, columns : Int, puzzle : Array[Array[Char]]) : Unit = {
        // Traverse through the 2D array
        for(x <- 0 until rows; y <- 0 until columns) {
            print(puzzle(x)(y))

            if(y + 1 == columns) {
                println()
            }
        }
    }

    def printHints(rows : Int, columns : Int, puzzle : Array[Array[Int]]) : Unit = {
        // Traverse through the 2D array
        for(x <- 0 until rows + columns; y <- 0 until rows + columns) {
            print(puzzle(x)(y) + " ")

            if(y + 1 == rows + columns) {
                println()
            }
        }
    }

    def getHints(rows : Int, columns : Int, scanner : Scanner) : Array[Array[Int]] = {
        var hints = Array.ofDim[Int](rows + columns, rows + columns)
        var hintString : String = scanner.useDelimiter("\\Z").next
        var num : Char = 0
        var y : Int = 0
        var index : Int = 0
        var character : Char = 0
        var strTrav : Int = 0

        character = hintString.charAt(index)

        for(x <- 0 until rows + columns) {
            y = 0
            while(character != 10 && index < hintString.length) {
                // Test if char read is an int
                if(character > 47 && character < 58 ) {
                    hints(x)(y) = character.asInstanceOf[Int] - 48
                    index += 1
                    y += 1
                    if(index < hintString.length) {
                        character = hintString.charAt(index)
                    }
                }
                // Test if char read is a space
                else if(character == ' ') {
                    index += 1
                    if(index < hintString.length) {
                        character = hintString.charAt(index)
                    }
                }
            }
            index += 1
            if(index < hintString.length) {
                character = hintString.charAt(index)
            }
        }

        return hints
    }

    def getNumHints(rowCol : Int, hints : Array[Array[Int]]) : Array[Int] = {
        var numHints : Int = 0
        var numHintsArr : Array[Int] = new Array[Int](rowCol)
        var y : Int = 0

        for(x <- 0 until rowCol) {
            y = 0
            numHints = 0
            while(hints(x)(y) != 0) {
                numHints += 1
                y += 1
            }
            numHintsArr(x) = numHints
        }

        return numHintsArr
    }

	def solvePuzzle(rows : Int, columns : Int) : Unit = {
        var puzzle = Array.ofDim[Char](rows, columns)
        puzzle = Array.fill(rows, columns){'.'}
        var totalCells = rows * columns
        var cellNum : Int = 0
        var pattern : Int = 0

        for(solution <- 0 until Math.pow(2, totalCells).toInt) {
            for(theRow <- 0 until rows) {
                for(theColumn <- 0 until columns) {
                    cellNum = (columns * theRow) + theColumn
                    pattern = (Math.pow(2, cellNum + 1)).toInt
                    puzzle(theRow)(theColumn) = (((solution % pattern) / (pattern / 2)) + 48).toChar
                }
            }
            printPuzzle(rows, columns, puzzle)
            println()
        }
	}

	// This confirms that the number of expected cells to be filled are
	def checkSolved(puzzle : Array[Array[Char]], hints : Array[Array[Int]], numHints : Array[Int], rows : Int, columns : Int) : Boolean = {
		var total : Int = 0
		var numFilled : Int = 0

		for(numHintsIndex <- 0 until numHints.length) {
			for(hintIndex <- 0 until numHints(numHintsIndex)) {
				total += hints(numHintsIndex)(hintIndex)
			}
			// If reading row hints
			if(numHintsIndex < rows) {
				for(puzzleColumn <- 0 until columns) {
					if(puzzle(numHintsIndex)(puzzleColumn) == 'X') {
						numFilled += 1
					}
				}
			}
			// If reading column hints
			else {
				for(puzzleRow <- 0 until rows) {
					if(puzzle(numHintsIndex)(puzzleRow) == 'X') {
						numFilled += 1
					}
				}
			}
			// Compare the expected number filled to what is filled
			if(numFilled != total) {
				return false
			}
		}
		return true
	}

}
