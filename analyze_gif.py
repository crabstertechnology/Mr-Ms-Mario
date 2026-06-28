from PIL import Image

im = Image.open("watcher-mochi/sd_content/happy.gif")
print("Format:", im.format)
print("Size:", im.size)
print("Frames:", im.n_frames)
print("Mode:", im.mode)
# Print first 20 pixels
im_rgba = im.convert("RGBA")
pixels = list(im_rgba.getdata())
print("Unique pixel colors (sample):", set(pixels[:100]))
