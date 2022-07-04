# Minesweeper Project #
Simple Minesweeper project based on a Slate plugin

## Info ##
A toolbar button opens a window with :

Content       | Description
------------- | -------------
Width         | Number of cells per row
Height        | Number of cells per column
Mines Number  | Number of mines per grid
Create Grid   | Generate the grid based on the prevoius setting

## Minesweeper Game ##

* Click on a bomb tile
    * Game Over
* Click on a non-bomb tile
    * The tile displays the number of adjacent bombs
    * Shows recursively the adjacent non-bomb tiles
* Reveal all non-bomb tiles avoiding all bombs
    * Game over
* Right click on a tile
    * Puts a flag on the tile (useful to mark the bombs), disabling all actions/inputs on that tile
* Right click on a flagged tile
    * Removes the flag from the tile and re-enables the inputs/actions on that tile

:small_blue_diamond: When game is over a message dialog allows the player to retry and start a new game

:small_blue_diamond: If the number of mines set is greater than the total number of cells, a message dialog warns the player of the invalid input and prevents the creation of the grid
