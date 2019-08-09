input_image_dir = "/path/to/image/dir"

num_parameters = 5
num_steps = 5

width = 960
height = 640
margin = 20

output_width = 1920

# ------------------------------------------------------------------------------

import subprocess

def generate_name(target_dim, step):
    name = "p"
    for dim in range(num_parameters):
        if target_dim == dim:
            name += "_" + f"{step * (1.0 / (num_steps - 1.0)):.2f}"
        else:
            name += "_0.50"
    return name + ".png"

def generate_key_and_files(target_dim):
    key = "p" + str(target_dim)
    names = []
    for step in range(num_steps):
        names.append(generate_name(target_dim, step))
    return key, names

image_sets = {}
for dim in range(num_parameters):
    key, names = generate_key_and_files(dim)
    image_sets[key] = names

for key, image_set in image_sets.items():
    num = len(image_set)

    out_width = width * num + margin * (num - 1)
    out_height = height
    out_image_path = "./" + key + ".png"

    subprocess.run(["convert", "-size", str(out_width) + "x" + str(out_height), "xc:none", out_image_path])

    for i in range(num):
        subprocess.run([
            "composite",
            "-dissolve",
            "100%",
            input_image_dir + "/" + image_set[i],
            out_image_path,
            "-geometry",
            "+" + str(i * (width + margin)) + "+0",
            "-matte",
            out_image_path
        ])

    final_image_path = "./" + key + ".jpg"

    subprocess.run(["convert", "-resize", str(output_width) + "x", "-background", "white", "-flatten", out_image_path, final_image_path])
