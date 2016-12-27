# Procedural Generation of Clouds with Vector Graphics.

This is just a quick experiment where I tried generating procedural
2D clouds using vector graphics. The experiment is just a program that
outputs a bunch of vector clouds as a SVG file:

![](img/montage.png)

You can see the SVG files in the directory `img/`.

## Building and Usage

You can build using make:

```
make
```

You can now generate an SVG with clouds by doing

```
./cloud_gen > out.svg
```

By changing the variable

```
int TYPE = 0;
```

in `main.cpp` you can generate different kinds of clouds.

## How Does this Work?

We start with the geometry for an ellipse:

<img src="img/step0.png" width="270" height="177" />

Then we replace every edge on the original ellipse with a cubic Bezier curve:

<img src="img/step1.png" width="270" height="177" />

To introduce some randomness, we randomly move the control points of
the cubic bezier curves some:

<img src="img/step2.png" width="270" height="177" />

and that's it!
