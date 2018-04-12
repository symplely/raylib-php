<?php

namespace raylib\Input;


class Mouse
{
    const LEFT_BUTTON = 0;
    const RIGHT_BUTTON = 1;
    const MIDDLE_BUTTON = 2;

    /**
     * Detect if a mouse button has been pressed once
     *
     * @param int $button
     *
     * @return bool
     */
    public static function isButtonPressed(int $button) : bool {}

    /**
     * Detect if a mouse button is being pressed
     *
     * @param int $button
     *
     * @return bool
     */
    public static function isButtonDown(int $button) : bool {}

    /**
     * Detect if a mouse button has been released once
     *
     * @param int $button
     *
     * @return bool
     */
    public static function isButtonReleased(int $button) : bool {}

    /**
     * Detect if a mouse button is NOT being pressed
     *
     * @param int $button
     *
     * @return bool
     */
    public static function isButtonUp(int $button) : bool {}

	/**
	* Returns mouse position X
	*
	* @return int
	*/
    public static function getX() : int {}

	/**
	* Returns mouse position Y
	*
	* @return int
	*/
    public static function getY() : int {}

	/**
	* Returns mouse position XY
	*
	* @return array The position of the mouse - Vector2 array ['x' => 0, 'y' => 0]
	*/
    public static function getPosition() : array {}

    /**
     * Set mouse position XY
     *
     * @param array $position The position to set the mouse - Vector2 array ['x' => 0, 'y' => 0]
     *
     * @return void
     */
    public static function setPosition(array $position) : void {}

    /**
     * Set mouse scaling
     *
     * @param float $scale
     *
     * @return void
     */
    public static function setScale(float $scale) : void {}

	/**
	* Returns mouse wheel movement Y
	*
	* @return void
	*/
    public static function getWheelMove() : int {}
}