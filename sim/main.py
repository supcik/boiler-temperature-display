import random

def values(request):
    with open("values.xml", 'r') as f:
        contents = f.read()
        return (
            contents.format(t=random.randrange(-80, 900)/10),
            200,
            {'Content-Type': 'application/xml'}
        )
