struct animal -> [
	chr *name;
	chr *speech;
];

typedef chr *ss;
x = 5;

extern fun int animal_create(animal *a, chr *name, chr *speech) -> [

	a->name = name;
	a->speech = speech;

	return 0;
]
