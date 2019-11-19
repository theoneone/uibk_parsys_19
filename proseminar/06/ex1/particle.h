#ifndef PARTICLE_Q_H
#define PARTICLE_Q_H

typedef struct quadtree {
	double x;
	double y;
	double mass;

	double v_x;
	double v_y;

	struct quadtree *parent;
	struct quadtree *north;
	struct quadtree *east;
	struct quadtree *south;
	struct quadtree *west;
} particle_t;

#endif /* PARTICLE_Q_H */
